/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/os/Sync.h>
#include <std/Testing.hpp>
#include <std/WorkerPool.hpp>
#include <std/os/Thread.hpp>

#include <thread>

struct TestParams {
  u32 x;
};

SN_TEST(Thread, createJoin) {
  auto func = [](void *arg) { (void)arg; };

  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = func,
      .param = nullptr,
  });
  CHECK(res.isOk());

  Optional<ThreadError> joinRes = res->join();
  CHECK(!joinRes.hasValue());
}

SN_TEST(Thread, params) {
  auto func = [](void *arg) {
    auto *params = reinterpret_cast<TestParams *>(arg);
    params->x = 1;
  };

  TestParams params = {.x = 0};
  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = func,
      .param = &params,
  });
  CHECK(res.isOk());

  Optional<ThreadError> joinRes = res->join();
  CHECK(!joinRes.hasValue());

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
  auto func = [](void *arg) {};
  Result<Thread, ThreadError> res = Thread::create({
      .entryPoint = func,
      .param = nullptr,
  });
  CHECK(res.isOk());

  Optional<ThreadError> joinRes = res->join();
  CHECK(!joinRes.hasValue());

  Optional<ThreadError> join2Res = res->join();
  CHECK(join2Res.hasValue());
  CHECK(join2Res.value() == ThreadError::AlreadyJoined);
}

SN_TEST(Thread, hardwareConcurrency) {
  u32 expected = std::thread::hardware_concurrency();
  u32 actual = Thread::hardwareConcurrency();
  CHECK(actual == expected);
}

SN_TEST(Barrier, createDestroy) {
  Arena::Scope temp = getScratch(nullptr, 0);
  Barrier *barrier = barrierCreate(temp, 4);
  CHECK(barrier != nullptr);
  barrierDestroy(barrier);
}

SN_TEST(Barrier, simple) {
  Arena::Scope temp = getScratch(nullptr, 0);

  struct ThreadArgs {
    Barrier *barrier;
    u32 *values;
    u32 index;
  };

  auto func = [](void *arg) {
    auto *args = reinterpret_cast<const ThreadArgs *>(arg);
    args->values[args->index] = 1;
    barrierSync(args->barrier);
  };

  u32 values[4] = {0, 0, 0, 0};
  Thread threads[4];
  ThreadArgs arrThreadArgs[4];

  Barrier *barrier = barrierCreate(temp, 5);

  for (u32 i = 0; i < 4; i++) {
    arrThreadArgs[i] = {barrier, values, i};
    Result<Thread, ThreadError> res = Thread::create({
        .entryPoint = func,
        .param = &arrThreadArgs[i],
    });
    threads[i] = res.unwrap();
  }

  barrierSync(barrier);

  for (u32 i = 0; i < 4; i++) {
    CHECK(values[i] == 1);
  }

  barrierDestroy(barrier);
}

SN_TEST(WorkerPool, one_thread) {
  Arena::Scope temp = getScratch(nullptr, 0);
  WorkerPool *wp = createWorkerPool(temp, 1);

  auto func = [](const Dispatch *D) {
    u32 &buf2 = D->parametersAs<u32>();
    u32 *buf = &buf2;
    buf[D->threadIndex.x] = D->threadIndex.x;
  };

  WorkContract *wc = wp->createWorkContract(temp, func);

  u32 buffer[32];
  for (u32 i = 0; i < 32; i++) {
    buffer[i] = 0xFFFFFFFF;
  }

  wp->dispatch(wc, buffer, 32);

  wp->release(wc);

  for (u32 i = 0; i < 32; i++) {
    CHECK(buffer[i] == i);
  }

  wp->shutdown();
}

SN_TEST(WorkerPool, multi_thread) {
  Arena::Scope temp = getScratch(nullptr, 0);
  WorkerPool *wp = createWorkerPool(temp, 8);

  auto func = [](const Dispatch *D) {
    u32 &buf2 = D->parametersAs<u32>();
    u32 *buf = &buf2;
    buf[D->threadIndex.x] = D->threadIndex.x;
  };

  WorkContract *wc = wp->createWorkContract(temp, func);

  u32 buffer[32];
  for (u32 i = 0; i < 32; i++) {
    buffer[i] = 0xFFFFFFFF;
  }

  wp->dispatch(wc, buffer, 32);

  wp->release(wc);

  for (u32 i = 0; i < 32; i++) {
    CHECK(buffer[i] == i);
  }

  wp->shutdown();
}

SN_TEST(WorkerPool, init) {
  Arena::Scope temp = getScratch(nullptr, 0);

  auto init = [](const Dispatch *D) {
    u32 &mask = D->parametersAs<u32>();
    mask |= u32(1) << D->idxPhysicalThread;
  };

  u32 mask = 0;
  const u32 expected = 0xF;

  WorkerPoolCreateInfo createInfo = {
      .numThreads = 4,
      .workerInitializer =
          WorkerPoolWorkerInitializer{
              .func = init,
              .parameters = &mask,
          },
  };
  WorkerPool *wp = createWorkerPool(temp, createInfo);

  wp->shutdown();

  CHECK(mask == expected);
}
