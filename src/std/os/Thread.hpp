/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "std/ChainedStruct.hpp"
#include "std/Optional.hpp"
#include "std/Result.hpp"
#include "std/Types.h"
#include "std/os/OsInfo.h"

enum class ThreadError {
  InsufficientResources,
  ValidationFailure,
  Deadlock,
  AlreadyJoined,
};

using ThreadEntryPoint = void (*)(void *arg);

struct ThreadCreateInfo {
  ChainedStruct *nextInChain = nullptr;

  ThreadEntryPoint entryPoint = nullptr;
  void *param = nullptr;
};

struct Thread {
  void *handle() const noexcept;
  Optional<ThreadError> join();

  static Result<Thread, ThreadError> create(const ThreadCreateInfo &info);

 private:
  void *_handle[2];
};
