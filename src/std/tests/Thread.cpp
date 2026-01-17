/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/Testing.hpp>
#include <std/os/Thread.hpp>

struct TestParams {
  u32 x;
};

SN_TEST(Thread, createJoin) {
  auto func = [](void *arg) -> void * {
    (void)arg;
    return nullptr;
  };

  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = func,
      .param = nullptr,
  });
  CHECK(res.isOk());

  Result<void *, ThreadError> joinRes = res->join();
  CHECK(joinRes.isOk());
}

static u32 THING = 0;

SN_TEST(Thread, joinResult) {
  auto func = [](void *arg) -> void * { return &THING; };

  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = func,
      .param = nullptr,
  });
  CHECK(res.isOk());

  Result<void *, ThreadError> joinRes = res->join();
  CHECK(joinRes.isOk());
  CHECK(joinRes.unwrap() == &THING);
}

SN_TEST(Thread, params) {
  auto func = [](void *arg) -> void * {
    auto *params = reinterpret_cast<TestParams *>(arg);
    params->x = 1;
    return nullptr;
  };

  TestParams params = {.x = 0};
  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = func,
      .param = &params,
  });
  CHECK(res.isOk());

  Result<void *, ThreadError> joinRes = res->join();
  CHECK(joinRes.isOk());

  CHECK(params.x == 1);
}

SN_TEST(Thread, nullFunc) {
  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = nullptr,
      .param = nullptr,
  });
  CHECK(res.isErr());
  CHECK(res.unwrapErr() == ThreadError::ValidationFailure);
}

SN_TEST(Thread, doubleJoin) {
  auto func = [](void *arg) -> void * { return nullptr; };
  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = func,
      .param = nullptr,
  });
  CHECK(res.isOk());

  Result<void *, ThreadError> joinRes = res->join();
  CHECK(joinRes.isOk());

  Result<void *, ThreadError> join2Res = res->join();
  CHECK(join2Res.isErr());
  CHECK(join2Res.unwrapErr() == ThreadError::AlreadyJoined);
}
