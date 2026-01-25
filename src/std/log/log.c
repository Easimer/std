/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "std/log.h"
#include "std/CompilerInfo.h"

#include <string.h>

#if EMSCRIPTEN
#include <emscripten/console.h>
#endif

#define MAX_CALLBACKS 32

typedef struct {
    log_LogFn fn;
    void *udata;
    int level;
} Callback;

static struct {
    void *udata;
    log_LockFn lock;
    int level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} L;


static const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


#if EMSCRIPTEN
static void stdout_callback(log_Event *ev) {
  char temp[1024];
  temp[vsnprintf(temp, 1023, ev->fmt, ev->ap)] = '\0';
  switch (ev->level)
  {
  case LOG_ERROR:
    emscripten_console_errorf("%s:%d: %s", ev->file, ev->line, temp);
    break;
  case LOG_WARN:
    emscripten_console_warnf("%s:%d: %s", ev->file, ev->line, temp);
    break;
  case LOG_TRACE:
    emscripten_console_tracef("%-5s %s:%d: %s", level_strings[ev->level], ev->file, ev->line, temp);
    break;
  default:
    emscripten_console_logf("%-5s %s:%d: %s", level_strings[ev->level], ev->file, ev->line, temp);
    break;
  }
}
#elif ANDROID

#include <string.h>
#include <android/log.h>

static android_LogPriority mapPrio(int level) {
    switch (level) {
        case LOG_TRACE:
            return ANDROID_LOG_DEBUG;
        case LOG_DEBUG:
            return ANDROID_LOG_DEBUG;
        case LOG_INFO:
            return ANDROID_LOG_INFO;
        case LOG_WARN:
            return ANDROID_LOG_WARN;
        case LOG_ERROR:
            return ANDROID_LOG_ERROR;
        case LOG_FATAL:
            return ANDROID_LOG_FATAL;
        default:
            return ANDROID_LOG_DEFAULT;
    }
}


static void stdout_callback(log_Event *ev) {
    android_LogPriority prio = mapPrio(ev->level);

    char tag[512];
    char message[512];
    memset(tag, 0, 512);
    memset(message, 0, 512);

    snprintf(tag, 511, "%s:%d: ", ev->file, ev->line);

    vsnprintf(message, 511, ev->fmt, ev->ap);

    __android_log_write(prio, tag, message);
}

#else

static void stdout_callback(log_Event *ev) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", &ev->time)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(
      ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
      buf, level_colors[ev->level], level_strings[ev->level],
      ev->file, ev->line);
#else
    fprintf(
            ev->udata, "%s %-5s %s:%d: ",
            buf, level_strings[ev->level], ev->file, ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

#endif


static void file_callback(log_Event *ev) {
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ev->time)] = '\0';
    fprintf(
            ev->udata, "%s %-5s %s:%d: ",
            buf, level_strings[ev->level], ev->file, ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}


static void lock(void) {
    if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void) {
    if (L.lock) { L.lock(false, L.udata); }
}


const char *log_level_string(int level) {
    return level_strings[level];
}


void log_set_lock(log_LockFn fn, void *udata) {
    L.lock = fn;
    L.udata = udata;
}


void log_set_level(int level) {
    L.level = level;
}


void log_set_quiet(bool enable) {
    L.quiet = enable;
}


int log_add_callback(log_LogFn fn, void *udata, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i] = (Callback) {fn, udata, level};
            return 0;
        }
    }
    return -1;
}

int log_del_callback(log_LogFn fn, void *udata, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (L.callbacks[i].fn == fn && L.callbacks[i].udata == udata) {
            L.callbacks[i].fn = NULL;
            return 0;
        }
    }
    return -1;
}

int log_add_fp(FILE *fp, int level) {
    return log_add_callback(file_callback, fp, level);
}


static void init_event(log_Event *ev, void *udata) {
    time_t t = time(NULL);
    memset(&ev->time, 0, sizeof(ev->time));
#if SN_MSVC
    localtime_s(&ev->time, &t);
#else
    localtime_r(&t, &ev->time);
#endif
    ev->udata = udata;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) {
    log_Event ev = {
            .fmt   = fmt,
            .file  = file,
            .line  = line,
            .level = level,
    };

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}
