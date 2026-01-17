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

void *Thread::handle() const noexcept {
  return _handle[0];
}

Optional<ThreadError> Thread::join() {
  if (_handle[0] == nullptr) {
    return ThreadError::AlreadyJoined;
  }

  pthread_t handle = reinterpret_cast<pthread_t>(_handle[0]);

  void *ret = nullptr;
  int rc = pthread_join(handle, &ret);

  if (rc == 0) {
    _handle[0] = nullptr;
    return Optional<ThreadError>();
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

#elif SN_STD_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// WakeByAddressSingle
#pragma comment(lib, "Synchronization.lib")

struct WrapperInfo {
  u32 flag;
  ThreadEntryPoint entry;
  void *arg;
};

static DWORD __stdcall entryPointWrapper(void *arg) {
  auto *pInfo = reinterpret_cast<WrapperInfo *>(arg);
  WrapperInfo info = *pInfo;

  pInfo->flag = 1;
  WakeByAddressSingle(arg);

  info.entry(info.arg);
  return 0;
}

void *Thread::handle() const noexcept {
  return _handle[0];
}

Optional<ThreadError> Thread::join() {
  if (_handle[0] == INVALID_HANDLE_VALUE) {
    return ThreadError::AlreadyJoined;
  }

  HANDLE handle = reinterpret_cast<HANDLE>(_handle[0]);

  DWORD rc = WaitForSingleObject(handle, INFINITE);

  if (rc == WAIT_OBJECT_0) {
    _handle[0] = INVALID_HANDLE_VALUE;
    return Optional<ThreadError>();
  }

  DWORD rc2 = GetLastError();
  (void)rc2;

  CHECK(!"Unexpected error");
  return ThreadError::ValidationFailure;
}

Result<Thread, ThreadError> Thread::create(const ThreadCreateInfo &info) {
  if (info.entryPoint == nullptr) {
    return ThreadError::ValidationFailure;
  }

  LPSECURITY_ATTRIBUTES securityAttributes = nullptr;
  SIZE_T dwStackSize = 0;
  WrapperInfo wrapperInfo = {0, info.entryPoint, info.param};
  DWORD id;
  HANDLE handle = CreateThread(securityAttributes, dwStackSize,
                               entryPointWrapper, &wrapperInfo, 0, &id);
  if (handle == nullptr) {
    DWORD rc = GetLastError();
    if (rc == ERROR_INVALID_PARAMETER) {
      return ThreadError::ValidationFailure;
    }

    CHECK(!"Unexpected error");
    return ThreadError::ValidationFailure;
  }

  u32 flagUnset = 0;
  WaitOnAddress(&wrapperInfo.flag, &flagUnset, sizeof(flagUnset), INFINITE);

  Thread ret;
  ret._handle[0] = (void *)handle;
  ret._handle[1] = nullptr;
  return ret;
}

#endif
