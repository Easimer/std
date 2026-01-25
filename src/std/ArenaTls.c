/*
 * Copyright (c) 2025 Daniel Meszaros
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "./Arena.h"
#include "./Check.h"
#include "./Types.h"

typedef struct {
  Arena *arena0;
  Arena *arena1;
} ThreadContext;

static ThreadContext* getCurrentThreadContext();

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static DWORD gTlsIndex = TLS_OUT_OF_INDEXES;

#define NUM_MAX_THREADS (4096)
static ThreadContext gThreadContextPool[NUM_MAX_THREADS];
static LONG gIdxNextThread = 0;

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL,
                    _In_ DWORD fdwReason,
                    _In_ LPVOID lpvReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
      // Allocate a TLS index for the per-thread context when the DLL is loaded
      CHECK(gTlsIndex == TLS_OUT_OF_INDEXES);
      gTlsIndex = TlsAlloc();
      CHECK(gTlsIndex != TLS_OUT_OF_INDEXES);
      break;
    }
    case DLL_PROCESS_DETACH: {
      // DLL is unloading
      if (lpvReserved != NULL) {
        // Process is being terminated
        break;
      }

      // Free the TLS index
      CHECK(gTlsIndex != TLS_OUT_OF_INDEXES);
      TlsFree(gTlsIndex);
      gTlsIndex = TLS_OUT_OF_INDEXES;
      break;
    }
  }

  return TRUE;
}

static ThreadContext *getCurrentThreadContext() {
  CHECK(gTlsIndex != TLS_OUT_OF_INDEXES);

  ThreadContext *S = (ThreadContext *)TlsGetValue(gTlsIndex);
  if (S == NULL) {
    LONG idxThread = InterlockedIncrement(&gIdxNextThread);
    DCHECK(idxThread < NUM_MAX_THREADS);
    S = &gThreadContextPool[idxThread];

    S->arena0 = S->arena1 = NULL;
    BOOL tlsSetOk = TlsSetValue(gTlsIndex, S);
    CHECK(tlsSetOk);
  }

  return S;
}
#else
#include <pthread.h>
#include <stdlib.h>

static pthread_key_t g_key;
static pthread_once_t g_once = PTHREAD_ONCE_INIT;

static void initKey() {
  int rc = pthread_key_create(&g_key, free);
  CHECK(rc == 0);
}

static ThreadContext* getCurrentThreadContext() {
  pthread_once(&g_once, initKey);

  ThreadContext* S = (ThreadContext*)pthread_getspecific(g_key);
  if (S == NULL) {
    S = (ThreadContext*)malloc(sizeof(ThreadContext));
    CHECK(S != NULL);
    S->arena0 = S->arena1 = NULL;
    int rc = pthread_setspecific(g_key, S);
    CHECK(rc == 0);
  }

  return S;
}
#endif

SN_STD_API ArenaTemp getScratch(Arena **pConflicts, u32 numConflicts) {
  ThreadContext* S = getCurrentThreadContext();

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
  ThreadContext* S = getCurrentThreadContext();
  S->arena0 = arena0;
  S->arena1 = arena1;
}

void SN_STD_WEAK_SYMBOL checkFail(const char *pExpr,
                                  const char *pFile,
                                  unsigned line) {
  (void)pExpr;
  (void)pFile;
  (void)line;
}
