/*
 * Copyright (c) 2026 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <std/Types.h>
#include <stddef.h>

enum ErrorFlagBits {
  ERROR_FLAG_NONE = 0,
  /** \brief An error of this type can be retried */
  ERROR_FLAG_RETRYABLE = 1 << 0,
};
typedef u32 ErrorFlags;

typedef struct ErrorType {
  /** \brief Human readable description of the error */
  const char *title;
  /** \brief Error flags */
  ErrorFlags flags;
} ErrorType;

typedef struct Error {
  /** \brief The type of this error */
  const ErrorType *type;
  /** \brief Human-readable details for this error */
  const char *detail;
  /** \brief A nullable pointer to an object that the error is related to */
  const void *instance;
  /** \brief System-specific error code */
  size_t status;
} Error;

/** \brief Expands to a format string for an Error; see also FMT_ERROR */
#define FMT_ERROR_FMTSTR "[\"%s\", detail=\"%s\", instance=%p, status=%zu]"

#define FMT_ERROR_GET_TITLE(Err) \
  (((Err).type) ? (Err).type->title : "(unknown error)")

/** \brief Supplies arguments when formatting an Error */
#define FMT_ERROR(Err) \
  FMT_ERROR_GET_TITLE(Err), ((Err).detail), ((Err).instance), ((Err).status)
