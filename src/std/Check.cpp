/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Check.h"
#include "std/log.h"

extern "C" void checkFail(const char *pExpr, const char *pFile, unsigned line) {
  log_fatal("\n  Assertion failed: %s\n    at %s:%u", pExpr, pFile, line);
  // TODO(danielm): print stacktrace
}
