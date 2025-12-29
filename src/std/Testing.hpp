#pragma once

#include "std/Arena.h"
#include "std/Check.h"
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
  extern "C" void testMain(Arena *arena0, Arena *arena1, SnTestStats *stats) { \
    setAllocatorsForThread(arena0, arena1);                                    \
                                                                               \
    SnTest *currentTest = gSnTestFirst;                                        \
    bool didPass;                                                              \
                                                                               \
    u32 numSuccess = 0;                                                        \
    u32 numTotal = 0;                                                          \
                                                                               \
    while (currentTest != nullptr) {                                           \
      printf("[%s] %s...", currentTest->suiteName, currentTest->name);         \
      didPass = false;                                                         \
                                                                               \
      if (!setjmp(gJmpBuf)) {                                                  \
        currentTest->pfnTest();                                                \
        /* Check leaks */                                                      \
        CHECK((arena0->end - arena0->beg) == SIZ_ARENA);                       \
        CHECK((arena1->end - arena1->beg) == SIZ_ARENA);                       \
        didPass = true;                                                        \
      }                                                                        \
      if (currentTest->shouldPass == didPass) {                                \
        printf("OK\n");                                                        \
        numSuccess += 1;                                                       \
      }                                                                        \
      numTotal += 1;                                                           \
      currentTest = currentTest->next;                                         \
    }                                                                          \
    stats->numSuccess = numSuccess;                                            \
    stats->numTotal = numTotal;                                                \
  }

extern "C" void testMain(Arena *arena0, Arena *arena1, SnTestStats *stats);
