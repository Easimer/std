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
static const Slice<char> DOT_STR = {DOT, 1};

static const char *SLASH = "/";
static const Slice<char> SLASH_STR = {SLASH, 1};

Slice<char> dirname(Slice<char> path) {
  while (path.length > 0 &&
         (path[path.length - 1] == '/' || path[path.length - 1] == '\\')) {
    // Skip trailing separators
    path.length -= 1;
  }

  if (path.length == 0) {
    return DOT_STR;
  }

  Optional<size_t> idxFwd = path.lastIndexOf('/');
  Optional<size_t> idxBack = path.lastIndexOf('\\');

  if (!idxFwd.hasValue() && !idxBack.hasValue()) {
    return DOT_STR;
  }

  size_t idxSlash = 0;
  if (idxFwd.hasValue() && idxBack.hasValue()) {
    idxSlash = *idxFwd > *idxBack ? *idxFwd : *idxBack;
  } else if (idxFwd.hasValue()) {
    idxSlash = *idxFwd;
  } else if (idxBack.hasValue()) {
    idxSlash = *idxBack;
  }

  if (idxSlash == 0) {
    return SLASH_STR;
  }

  return {path.data, idxSlash};
}

Slice<char> basename(Slice<char> path) {
  while (path.length > 0 &&
         (path[path.length - 1] == '/' || path[path.length - 1] == '\\')) {
    // Skip trailing separators
    path.length -= 1;
  }

  if (path.length == 0) {
    return {};
  }

  Optional<size_t> idxFwd = path.lastIndexOf('/');
  Optional<size_t> idxBack = path.lastIndexOf('\\');

  if (!idxFwd.hasValue() && !idxBack.hasValue()) {
    return path;
  }

  size_t idxSlash = 0;
  if (idxFwd.hasValue() && idxBack.hasValue()) {
    idxSlash = *idxFwd > *idxBack ? *idxFwd : *idxBack;
  } else if (idxFwd.hasValue()) {
    idxSlash = *idxFwd;
  } else if (idxBack.hasValue()) {
    idxSlash = *idxBack;
  }

  return path.subarray(idxSlash + 1);
}

MutSlice<char> joinSimple(Arena *arena,
                       Slice<char> segment0,
                       Slice<char> segment1) {
  MutSlice<char> ret;
  if (segment0.length == 0) {
    if (segment1.length == 0) {
      return {nullptr, 0};
    }

    allocNZ(arena, segment1.length, ret);
    ret.copy(segment1);
    return ret;
  }

  char lastCharSeg0 = segment0[segment0.length - 1];
  bool needSeparator = lastCharSeg0 != '/' && lastCharSeg0 != '\\';
  size_t lenRet = segment0.length + (needSeparator ? 1 : 0) + segment1.length;

  allocNZ(arena, lenRet, ret);
  MutSlice<char> segment0Out, segment1Out;

  segment0Out = ret.subarray(0, segment0.length);
  if (needSeparator) {
    segment1Out = ret.subarray(segment0.length + 1);
    ret[segment0.length] = '/';
  } else {
    segment1Out = ret.subarray(segment0.length);
  }

  segment0Out.copy(segment0);
  segment1Out.copy(segment1);
  return ret;
}
