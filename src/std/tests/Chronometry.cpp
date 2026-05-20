#include <std/Check.h>
#include <std/Chronometry.h>
#include <std/Testing.hpp>

#include <stdio.h>

SN_TEST(Chronometry, getCurrentTimeExecutes) {
  TimePoint res = chrono_getCurrentTime();
  (void)res;
  CHECK(1);
}

SN_TEST(Chronometry, secondsBetween) {
  TimePoint t0 = chrono_getCurrentTime();
  chrono_sleep(1);
  TimePoint t1 = chrono_getCurrentTime();
  f64 elapsed = chrono_secondsBetween(t0, t1);
  printf("elapsed %f\n", elapsed);
  CHECK(0 <= elapsed && elapsed <= 2.0f);
}

SN_TEST(Chronometry, get_local_date) {
  struct chrono_date d = chrono_get_local_date();
  (void)d;
  CHECK(1);
}
