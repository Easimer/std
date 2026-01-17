/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <std/os/Thread.hpp>

#if SN_STD_SYSTEM_HAS_PTHREADS
#include <errno.h>
#include <pthread.h>

Result<void *, ThreadError> Thread::join() {
  if (_handle[0] == nullptr) {
    return ThreadError::AlreadyJoined;
  }

  pthread_t handle = reinterpret_cast<pthread_t>(_handle[0]);

  void *ret = nullptr;
  int rc = pthread_join(handle, &ret);

  if (rc == 0) {
    _handle[0] = nullptr;
    return ret;
  }

  if (rc == EDEADLK) {
    return ThreadError::Deadlock;
  } else if (rc == EINVAL) {
    return ThreadError::ValidationFailure;
  } else if (rc == ESRCH) {
    return ThreadError::ValidationFailure;
  }

  CHECK(!"Unexpected error");
  return ThreadError::ValidationFailure;
}

Result<Thread, ThreadError> Thread::create(const ThreadCreateInfo &info) {
  if (info.entryPoint == nullptr) {
    return ThreadError::ValidationFailure;
  }

  pthread_t handle;
  int rc = pthread_create(&handle, nullptr, info.entryPoint, info.param);

  Thread ret;
  ret._handle[0] = (void *)handle;
  ret._handle[1] = nullptr;
  if (rc == 0) {
    return ret;
  }

  if (rc == EAGAIN) {
    return ThreadError::InsufficientResources;
  } else if (rc == EINVAL || rc == EPERM) {
    return ThreadError::ValidationFailure;
  }

  CHECK(!"Unexpected error");
  return ThreadError::ValidationFailure;
}

#endif
