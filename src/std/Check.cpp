/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Check.h"
#include "std/CompilerInfo.h"
#include "std/log.h"

static void checkFailDefaultImpl(const char *expr,
                                 const char *file,
                                 unsigned line) {
  log_fatal("\n  Assertion failed: %s\n    at %s:%u", expr, file, line);
  // TODO(danielm): print stacktrace
}

#if SN_MSVC

// If the LINK can't find checkFail, use checkFailDefault
#pragma comment(linker, "/alternatename:checkFail=checkFailDefault")
extern "C" void checkFailDefault(const char *expr,
                                 const char *file,
                                 unsigned line) {
  checkFailDefaultImpl(expr, file, line);
}

#else  /* !SN_MSVC */
extern "C" SN_WEAK_FUNCTION void checkFail(const char *expr,
                                           const char *file,
                                           unsigned line) {
  checkFailDefaultImpl(expr, file, line);
}
#endif /* !SN_MSVC */
