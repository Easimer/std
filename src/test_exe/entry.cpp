/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/Arena.h>
#include <std/Check.h>
#include <std/Chronometry.h>
#include <std/log.h>
#include <std/Slice.hpp>
#include <std/SliceUtils.hpp>
#include <std/Testing.hpp>

#include <stdio.h>
#include <stdlib.h>

SnTest *gSnTestFirst = nullptr;
SnTest *gSnTestPrev = nullptr;
static std::jmp_buf gJmpBuf;
static bool gSnRunningInGA = false;
static Slice<char> gSnCwd;

Slice<char> getWorkingDir(Slice<char> dst, u32 &sizRequired);

#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

Slice<char> getWorkingDir(Slice<char> dst, u32 &sizRequired) {
  DWORD rc = GetCurrentDirectoryA(dst.length, dst.data);
  if (rc == 0) {
    sizRequired = 0;
    return {};
  }

  sizRequired = rc;
  return dst.subarray(0, rc);
}

#else
#include <unistd.h>
Slice<char> getWorkingDir(Slice<char> dst, u32 &sizRequired) {
  char *res = getcwd(dst.data, dst.length);
  if (res == nullptr) {
    sizRequired = dst.length != 0 ? dst.length * 2 : 1;
    return {};
  }

  u32 len = strlen(res);
  sizRequired = len;
  return dst.subarray(0, len);
}
#endif

static const u32 SIZ_ARENA = 256 * 1024;

static Arena makeArena() {
  u8 *base = reinterpret_cast<u8 *>(malloc(SIZ_ARENA));
  u8 *end = base + SIZ_ARENA;
  return {base, end};
}

static Slice<const char> getRepoRelativePath(Slice<const char> file) {
  if (!file.startsWith(gSnCwd.asConst())) {
    return file;
  }

  return file.subarray(gSnCwd.length + 1);
}

extern "C" void checkFail(const char *pExpr, const char *pFile, unsigned line) {
  if (gSnRunningInGA) {
    Slice<const char> pathFileRelRepo =
        getRepoRelativePath({pFile, (u32)strlen(pFile)});
    printf("::error file=%.*s,line=%u::Assertion failed: %s\n",
           FMT_SLICE(pathFileRelRepo), line, pExpr);
  } else {
    log_fatal("\n  Assertion failed: %s\n    at %s:%u\n", pExpr, pFile, line);
  }
  std::longjmp(gJmpBuf, 1);
}

extern "C" void handleOOM(Arena *arena) {
  log_fatal("\n  Arena %p is out of memory", arena);
  std::longjmp(gJmpBuf, 1);
}

extern "C" SnTestResult *testMain(Arena *arena0,
                                  Arena *arena1,
                                  Arena *arenaResults,
                                  SnTestStats *stats) {
  SnTestResult *res = nullptr;
  SnTestResult *ret = nullptr;
  setAllocatorsForThread(arena0, arena1);

  SnTest *currentTest = gSnTestFirst;
  bool didPass = false;

  u32 numSuccess = 0;
  u32 numTotal = 0;
  TimePoint t_start = chrono_getCurrentTime();
  TimePoint t_end;
  SnTestResult *prev;

  while (currentTest != nullptr) {
    prev = res;
    res = alloc<SnTestResult>(arenaResults);
    if (prev != nullptr) {
      prev->next = res;
    } else {
      ret = res;
    }
    res->next = nullptr;
    res->test = currentTest;
    res->ok = false;
    res->duration = 0;

    didPass = false;

    const SnTestMetadata *meta = currentTest->metadata;
    if (gSnRunningInGA) {
      printf("::group::[%s] %s\n", meta->suiteName, meta->name);
    }

    if (!setjmp(gJmpBuf)) {
      t_start = chrono_getCurrentTime();
      currentTest->pfnTest();
      // Check for leaks
      CHECK((arena0->end - arena0->beg) == SIZ_ARENA);
      CHECK((arena1->end - arena1->beg) == SIZ_ARENA);
      didPass = true;
    }
    t_end = chrono_getCurrentTime();
    if (currentTest->shouldPass == didPass) {
      res->ok = true;
      numSuccess += 1;
    } else {
      if (!gSnRunningInGA) {
        printf("=== FAILED: [%s] %s ===\n", meta->suiteName, meta->name);
        printf("  Defined at %s:%d\n", meta->file, meta->line);
        printf("===================\n");
      }
    }
    res->duration = chrono_secondsBetween(t_start, t_end);
    numTotal += 1;
    currentTest = currentTest->next;
    if (gSnRunningInGA) {
      printf("::endgroup\n");
    }
  }
  stats->numSuccess = numSuccess;
  stats->numTotal = numTotal;
  return ret;
}

static void printRes(const SnTestResult *results) {
  if (results->ok) {
    printf("{  OK  } ");
  } else {
    printf("{ FAIL } ");
  }

  const SnTestMetadata *meta = results->test->metadata;
  printf("[%s] %s [%.03f ms]\n", meta->suiteName, meta->name,
         results->duration * 1000.0);
}

static void printfMarkdownRow(FILE *f,
                              const SnTestResult *cur,
                              const char *baseUrl) {
  const SnTestMetadata *meta = cur->test->metadata;
  Slice<const char> pathFileRelRepo =
      getRepoRelativePath({meta->file, (u32)strlen(meta->file)});

  char url[1024];
  snprintf(url, sizeof(url) - 1, "%s%.*s#L%d", baseUrl,
           FMT_SLICE(pathFileRelRepo), meta->line);
  url[1023] = '\0';

  f64 duration_ms = cur->duration * 1000.0;

  fprintf(f, "| `%s` | `%s` | %f ms | [%.*s#L%d](%s) |\n", meta->suiteName,
          meta->name, duration_ms, FMT_SLICE(pathFileRelRepo), meta->line, url);
}

static void generateActionsJobSummary(const SnTestStats *stats,
                                      SnTestResult *results) {
  const char *pathSummary = getenv("GITHUB_STEP_SUMMARY");
  const char *serverUrl = getenv("GITHUB_SERVER_URL");
  const char *repository = getenv("GITHUB_REPOSITORY");
  const char *commit = getenv("GITHUB_SHA");
  if (pathSummary == nullptr || serverUrl == nullptr || repository == nullptr ||
      commit == nullptr) {
    return;
  }

  char baseUrl[256];
  snprintf(baseUrl, 255, "%s/%s/tree/%s/", serverUrl, repository, commit);

  FILE *f = fopen(pathSummary, "wb");
  if (f == nullptr) {
    return;
  }

  u32 numFailed = stats->numTotal - stats->numSuccess;

  fprintf(f, "# %s\n", repository);
  fprintf(f, "|  |  |\n");
  fprintf(f, "| --- | --- |\n");
  fprintf(f, "| **Successful tests** | %u |\n", stats->numSuccess);
  fprintf(f, "| **Failed tests** | %u |\n", numFailed);
  fprintf(f, "| **Total** | %u |\n", stats->numTotal);

  if (numFailed != 0) {
    fprintf(f, "## Failed tests\n");
    fprintf(f, "| Suite | Name | Duration | Definition |\n");
    fprintf(f, "| --- | --- | --- | --- |\n");
    SnTestResult *cur = results;
    while (cur != nullptr) {
      if (cur->ok) {
        cur = cur->next;
        continue;
      }

      printfMarkdownRow(f, cur, baseUrl);

      cur = cur->next;
    }
  }

  {
    fprintf(f, "## Successful tests\n");
    fprintf(f, "| Suite | Name | Duration | Definition |\n");
    fprintf(f, "| --- | --- | --- | --- |\n");
    SnTestResult *cur = results;
    while (cur != nullptr) {
      if (!cur->ok) {
        cur = cur->next;
        continue;
      }

      printfMarkdownRow(f, cur, baseUrl);

      cur = cur->next;
    }
  }

  fclose(f);
}

int main(int numArgs, char **arrArgs) {
  Arena arena0 = makeArena();
  Arena arena1 = makeArena();
  Arena arenaResults = makeArena();

  Slice<char> cwd;
  u32 sizRequired = 1;
  while (cwd.length < sizRequired) {
    Arena saved = arenaResults;
    alloc(&arenaResults, sizRequired, cwd);
    cwd = getWorkingDir(cwd, sizRequired);

    if (cwd.length < sizRequired) {
      arenaResults = saved;
    }
  };

  gSnCwd = cwd;
  printf("Working directory: %.*s\n", FMT_SLICE(cwd));

  const char *ga = getenv("CI");
  if (ga != nullptr) {
    gSnRunningInGA = true;
  }

  printf("Running in GA: %d\n", gSnRunningInGA ? 1 : 0);

  SnTestStats stats;
  TimePoint t_start = chrono_getCurrentTime();
  SnTestResult *const results =
      testMain(&arena0, &arena1, &arenaResults, &stats);
  TimePoint t_end = chrono_getCurrentTime();

  u32 numFail = stats.numTotal - stats.numSuccess;
  f64 percentSuccess = stats.numSuccess / f64(stats.numTotal) * 100.0;
  f64 percentFail = numFail / f64(stats.numTotal) * 100.0;

  printf("\n");
  if (gSnRunningInGA) {
    printf("::group::Summary\n");
  }

  if (numFail != 0) {
    // Print failed tests first
    SnTestResult *cur = results;
    while (cur != nullptr) {
      if (cur->ok) {
        cur = cur->next;
        continue;
      }

      printRes(cur);
      cur = cur->next;
    }
    printf("\n");
  }

  {
    SnTestResult *cur = results;
    while (cur != nullptr) {
      if (!cur->ok) {
        cur = cur->next;
        continue;
      }

      printRes(cur);
      cur = cur->next;
    }
    printf("\n");
  }

  printf("Successful tests: %u/%u (%.2f%%)\n", stats.numSuccess, stats.numTotal,
         percentSuccess);
  printf("Failed tests:     %u/%u (%.2f%%)\n", numFail, stats.numTotal,
         percentFail);

  if (gSnRunningInGA) {
    generateActionsJobSummary(&stats, results);
  }

  free(arenaResults.beg);
  free(arena1.beg);
  free(arena0.beg);

  f64 elapsed = chrono_secondsBetween(t_start, t_end);
  printf("Duration:         %f milliseconds\n", elapsed * 1000.0);

  if (gSnRunningInGA) {
    printf("::endgroup\n");
  }

  int rc = (numFail == 0) ? 0 : 1;
  return rc;
}
