#pragma once

#include "std/Arena.h"
#include "std/Check.h"
#include "std/Chronometry.h"
#include "std/Types.h"

#include <csetjmp>

#ifndef SN_TEST_EXECUTABLE
#define SN_TEST_EXECUTABLE 0
#endif

struct SnTest;
using SnTestFunc = void (*)(void);
extern SnTest *gSnTestFirst;
extern SnTest *gSnTestPrev;

struct SnTest {
  SnTest *next = nullptr;
  const char *suiteName;
  const char *name;
  bool shouldPass;

  void (*pfnTest)(void);

  SnTest(const char *suiteName,
         const char *name,
         void (*pfnTest)(void),
         bool shouldPass)
      : next(nullptr),
        suiteName(suiteName),
        name(name),
        shouldPass(shouldPass),
        pfnTest(pfnTest) {
    if (gSnTestFirst != nullptr) {
      gSnTestPrev->next = this;
      gSnTestPrev = this;
    } else {
      gSnTestFirst = gSnTestPrev = this;
    }
  }
};

struct SnTestStats {
  u32 numSuccess;
  u32 numTotal;
};

struct SnTestResult {
  SnTestResult *next;
  const SnTest *test;
  f64 duration;
  bool ok;
};

#define SN_TEST_STRINGIFY2(X) #X
#define SN_TEST_STRINGIFY(X) SN_TEST_STRINGIFY2(X)

/** \brief Declares a test function */
#define SN_TEST_DECL_FUNC(SuiteName, TestName) \
  static void test_func_##SuiteName##_##TestName(void)

#if SN_TEST_EXECUTABLE
/** \brief Creates a test definition */
#define SN_TEST_DEFINE_DESC(SuiteName, TestName, ShouldPass)            \
  static SnTest test_##SuiteName##_##TestName =                         \
      SnTest(SN_TEST_STRINGIFY(SuiteName), SN_TEST_STRINGIFY(TestName), \
             test_func_##SuiteName##_##TestName, ShouldPass)
#else
#define SN_TEST_DEFINE_DESC(SuiteName, TestName, ShouldPass)
#endif

/** \brief Defines a test that must pass. */
#define SN_TEST(SuiteName, TestName)              \
  SN_TEST_DECL_FUNC(SuiteName, TestName);         \
  SN_TEST_DEFINE_DESC(SuiteName, TestName, true); \
  static void test_func_##SuiteName##_##TestName(void)

/** \brief Defines a test that must fail. */
#define SN_TEST_MUST_FAIL(SuiteName, TestName)     \
  SN_TEST_DECL_FUNC(SuiteName, TestName);          \
  SN_TEST_DEFINE_DESC(SuiteName, TestName, false); \
  static void test_func_##SuiteName##_##TestName(void)

#define SN_TEST_MAIN                                                           \
  SnTest *gSnTestFirst = nullptr;                                              \
  SnTest *gSnTestPrev = nullptr;                                               \
  static std::jmp_buf gJmpBuf;                                                 \
  extern "C" void checkFail(const char *pExpr, const char *pFile,              \
                            unsigned line) {                                   \
    log_fatal("\n  Assertion failed: %s\n    at %s:%u\n", pExpr, pFile, line); \
    std::longjmp(gJmpBuf, 1);                                                  \
  }                                                                            \
                                                                               \
  extern "C" void handleOOM(Arena *arena) {                                    \
    log_fatal("\n  Arena %p is out of memory", arena);                         \
    std::longjmp(gJmpBuf, 1);                                                  \
  }                                                                            \
  extern "C" SnTestResult *testMain(Arena *arena0, Arena *arena1,              \
                                    Arena *arenaResults, SnTestStats *stats) { \
    SnTestResult *res = nullptr;                                               \
    SnTestResult *ret = nullptr;                                               \
    setAllocatorsForThread(arena0, arena1);                                    \
                                                                               \
    SnTest *currentTest = gSnTestFirst;                                        \
    bool didPass = false;                                                      \
                                                                               \
    u32 numSuccess = 0;                                                        \
    u32 numTotal = 0;                                                          \
    TimePoint t_start;                                                         \
    TimePoint t_end;                                                           \
    SnTestResult *prev;                                                        \
                                                                               \
    while (currentTest != nullptr) {                                           \
      prev = res;                                                              \
      res = alloc<SnTestResult>(arenaResults);                                 \
      if (prev != nullptr) {                                                   \
        prev->next = res;                                                      \
      } else {                                                                 \
        ret = res;                                                             \
      }                                                                        \
      res->next = nullptr;                                                     \
      res->test = currentTest;                                                 \
      res->ok = false;                                                         \
      res->duration = 0;                                                       \
                                                                               \
      didPass = false;                                                         \
                                                                               \
      if (!setjmp(gJmpBuf)) {                                                  \
        t_start = chrono_getCurrentTime();                                     \
        currentTest->pfnTest();                                                \
        /* Check leaks */                                                      \
        CHECK((arena0->end - arena0->beg) == SIZ_ARENA);                       \
        CHECK((arena1->end - arena1->beg) == SIZ_ARENA);                       \
        didPass = true;                                                        \
      }                                                                        \
      t_end = chrono_getCurrentTime();                                         \
      if (currentTest->shouldPass == didPass) {                                \
        res->ok = true;                                                        \
        numSuccess += 1;                                                       \
      } else {                                                                 \
        printf("=== FAILED: [%s] %s ===\n", currentTest->suiteName,            \
               currentTest->name);                                             \
      }                                                                        \
      res->duration = chrono_secondsBetween(t_start, t_end);                   \
      numTotal += 1;                                                           \
      currentTest = currentTest->next;                                         \
    }                                                                          \
    stats->numSuccess = numSuccess;                                            \
    stats->numTotal = numTotal;                                                \
    return ret;                                                                \
  }

extern "C" SnTestResult *testMain(Arena *arena0,
                                  Arena *arena1,
                                  Arena *arenaResults,
                                  SnTestStats *stats);
