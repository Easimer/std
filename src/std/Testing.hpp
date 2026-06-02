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

struct SnTestMetadata {
  const char *suiteName;
  const char *name;

  const char *file;
  int line;
};

struct SnTest {
  SnTest *next;
  const SnTestMetadata *metadata;
  bool shouldPass;

  void (*pfnTest)(void);

  SnTest(const SnTestMetadata *metadata, void (*pfnTest)(void), bool shouldPass)
      : next(nullptr),
        metadata(metadata),
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
#define SN_TEST_DEFINE_DESC(SuiteName, TestName, ShouldPass)         \
  static const SnTestMetadata test_meta_##SuiteName##_##TestName = { \
      .suiteName = SN_TEST_STRINGIFY(SuiteName),                     \
      .name = SN_TEST_STRINGIFY(TestName),                           \
      .file = __FILE__,                                              \
      .line = __LINE__,                                              \
  };                                                                 \
  static SnTest test_##SuiteName##_##TestName =                      \
      SnTest(&test_meta_##SuiteName##_##TestName,                    \
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

