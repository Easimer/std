#pragma once

#include "./Arena.h"
#include "./Check.h"
#include "./Chronometry.h"
#include "./Types.h"

struct SnTest;
struct SnTestMetadata;
typedef void (*SnTestFunc)(void);

struct SnTestChain {
  struct SnTestChain *next;
};

#if __cplusplus
extern "C" {
#endif

SN_STD_API void snTestRegister(struct SnTestChain *test);
SN_STD_API struct SnTestChain *snTestGetFirst(void);

#if __cplusplus
}
#endif

#if __cplusplus

struct SnTestMetadata {
  const char *suiteName;
  const char *name;

  const char *file;
  int line;
};

struct SnTest {
  struct SnTestChain chain;

  const struct SnTestMetadata *metadata;
  bool shouldPass;

  void (*pfnTest)(void);

  SnTest(const SnTestMetadata *metadata, void (*pfnTest)(void), bool shouldPass)
      : chain({nullptr}),
        metadata(metadata),
        shouldPass(shouldPass),
        pfnTest(pfnTest) {
    snTestRegister(&chain);
  }
};

static inline SnTest *snTestFrom(SnTestChain *node) {
  return reinterpret_cast<SnTest *>(node);
}

static inline SnTest *snTestNext(SnTest *test) {
  return snTestFrom(test->chain.next);
}

struct SnTestStats {
  u32 numSuccess;
  u32 numTotalExecuted;

  u32 numSkipped;
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

#endif

#define ASSERT_EQUAL(Actual, Expected) \
  CHECK_EX((Actual) == (Expected),     \
           "Expected `" #Actual "' to equal `" #Expected "'");

#define ASSERT_NOT_EQUAL(Actual, Expected) \
  CHECK_EX((Actual) != (Expected),         \
           "Expected `" #Actual "' not to equal `" #Expected "'");

#define ASSERT_IS_TRUE(Actual) \
  CHECK_EX((Actual), "Expected `" #Actual "' to be true");

#define ASSERT_IS_FALSE(Actual) \
  CHECK_EX(!(Actual), "Expected `" #Actual "' to be false");

/**
 * \brief Asserts that Actual is between Min and Max (inclusive). The expression
 * `Actual` will be evaluated twice!
 */
#define ASSERT_BETWEEN(Actual, Min, Max)           \
  CHECK_EX((Min) <= (Actual) && (Actual) <= (Max), \
           "Expected `" #Actual "' to be between `" #Min "' and `" #Max "'");
