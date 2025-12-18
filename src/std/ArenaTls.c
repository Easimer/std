/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "std/Check.h"
#include "std/Arena.h"

#include <stdlib.h>

typedef struct {
  Arena *arena0;
  Arena *arena1;
} ThreadState;

static ThreadState* getCurrentThreadState();

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
static DWORD idxTls = TLS_OUT_OF_INDEXES;

static ThreadState* getCurrentThreadState() {
  if(idxTls == TLS_OUT_OF_INDEXES) {
    idxTls = TlsAlloc();
    CHECK(idxTls != TLS_OUT_OF_INDEXES);
  }

  ThreadState* S = (ThreadState*)TlsGetValue(idxTls);
  if (S == NULL) {
    S = (ThreadState*)malloc(sizeof(ThreadState));
    CHECK(S != NULL);
    S->arena0 = S->arena1 = NULL;
    BOOL tlsSetOk = TlsSetValue(idxTls, S);
    CHECK(tlsSetOk);
  }
  
  return S;
}
#else
#include <pthread.h>

static pthread_key_t g_key;
static pthread_once_t g_once = PTHREAD_ONCE_INIT;

static void initKey() {
  int rc = pthread_key_create(&g_key, free);
  CHECK(rc == 0);
}

static ThreadState* getCurrentThreadState() {
  pthread_once(&g_once, initKey);

  ThreadState* S = (ThreadState*)pthread_getspecific(g_key);
  if (S == NULL) {
    S = (ThreadState*)malloc(sizeof(ThreadState));
    CHECK(S != NULL);
    S->arena0 = S->arena1 = NULL;
    int rc = pthread_setspecific(g_key, S);
    CHECK(rc == 0);
  }

  return S;
}
#endif

SN_STD_API ArenaTemp getScratch(Arena **pConflicts, u32 numConflicts) {
  ThreadState* S = getCurrentThreadState();

  if(numConflicts == 0) {
    return saveArena(S->arena0);
  }

  DCHECK(numConflicts == 1);

  Arena* conflict = pConflicts[0];
  if(S->arena0 == conflict) {
    return saveArena(S->arena1);
  } else {
    return saveArena(S->arena0);
  }
}

SN_STD_API void setAllocatorsForThread(Arena *arena0, Arena* arena1) {
  ThreadState* S = getCurrentThreadState();
  S->arena0 = arena0;
  S->arena1 = arena1;
}

void checkFail(const char *pExpr, const char *pFile, unsigned line) {}
