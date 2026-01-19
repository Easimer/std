/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Path.hpp"
#include "std/Slice.hpp"
#include "std/SliceUtils.hpp"

static const char *DOT = ".";
static const Slice<const char> DOT_STR = {DOT, 1};

static const char *SLASH = "/";
static const Slice<const char> SLASH_STR = {SLASH, 1};

Slice<const char> dirname(Slice<const char> path) {
  while (path.length > 0 &&
         (path[path.length - 1] == '/' || path[path.length - 1] == '\\')) {
    // Skip trailing separators
    path.length -= 1;
  }

  if (path.length == 0) {
    return DOT_STR;
  }

  u32 idxFwd;
  u32 idxBack;
  bool hasFwd = lastIndexOf<const char>(path, '/', &idxFwd);
  bool hasBack = lastIndexOf<const char>(path, '\\', &idxBack);

  if (!hasFwd && !hasBack) {
    return DOT_STR;
  }

  u32 idxSlash = 0;
  if (hasFwd & hasBack) {
    idxSlash = idxFwd > idxBack ? idxFwd : idxBack;
  } else if (hasFwd) {
    idxSlash = idxFwd;
  } else if (hasBack) {
    idxSlash = idxBack;
  } else {
    NOTREACHED();
  }

  if (idxSlash == 0) {
    return SLASH_STR;
  }

  return {path.data, idxSlash};
}

Slice<const char> basename(Slice<const char> path) {
  while (path.length > 0 &&
         (path[path.length - 1] == '/' || path[path.length - 1] == '\\')) {
    // Skip trailing separators
    path.length -= 1;
  }

  if (path.length == 0) {
    return {nullptr, 0};
  }
  u32 idxFwd, idxBack;
  bool hasFwd = lastIndexOf<const char>(path, '/', &idxFwd);
  bool hasBack = lastIndexOf<const char>(path, '\\', &idxBack);

  if (!hasFwd && !hasBack) {
    return path;
  }

  u32 idxSlash = 0;
  if (hasFwd & hasBack) {
    idxSlash = idxFwd > idxBack ? idxFwd : idxBack;
  } else if (hasFwd) {
    idxSlash = idxFwd;
  } else if (hasBack) {
    idxSlash = idxBack;
  } else {
    NOTREACHED();
  }

  return subarray(path, idxSlash + 1);
}

Slice<char> joinSimple(Arena *arena,
                       Slice<const char> segment0,
                       Slice<const char> segment1) {
  Slice<char> ret;
  if (segment0.length == 0) {
    if (segment1.length == 0) {
      return {nullptr, 0};
    }

    allocNZ(arena, segment1.length, ret);
    copy(ret, segment1);
    return ret;
  }

  char lastCharSeg0 = segment0[segment0.length - 1];
  bool needSeparator = lastCharSeg0 != '/' && lastCharSeg0 != '\\';
  u32 lenRet = segment0.length + (needSeparator ? 1 : 0) + segment1.length;

  allocNZ(arena, lenRet, ret);
  Slice<char> segment0Out, segment1Out;

  segment0Out = subarray(ret, 0, segment0.length);
  if (needSeparator) {
    segment1Out = subarray(ret, segment0.length + 1);
    ret[segment0.length] = '/';
  } else {
    segment1Out = subarray(ret, segment0.length);
  }

  copy(segment0Out, segment0);
  copy(segment1Out, segment1);
  return ret;
}
