/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
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

#include <assert.h>

#include "uv.h"
#include "internal.h"


int uv_is_active(uv_handle_t* handle) {
  switch (handle->type) {
    case UV_TIMER:
    case UV_IDLE:
    case UV_PREPARE:
    case UV_CHECK:
      return (handle->flags & UV_HANDLE_ACTIVE) ? 1 : 0;

    default:
      return 1;
  }
}


/* TODO: integrate this with uv_close. */
static void uv_close_error(uv_handle_t* handle, uv_err_t e) {
  uv_tcp_t* tcp;
  uv_pipe_t* pipe;
  uv_process_t* process;

  if (handle->flags & UV_HANDLE_CLOSING) {
    return;
  }

  handle->error = e;
  handle->flags |= UV_HANDLE_CLOSING;

  /* Handle-specific close actions */
  switch (handle->type) {
    case UV_TCP:
      tcp = (uv_tcp_t*)handle;
      /* If we don't shutdown before calling closesocket, windows will */
      /* silently discard the kernel send buffer and reset the connection. */
      if (!(tcp->flags & UV_HANDLE_SHUT)) {
        shutdown(tcp->socket, SD_SEND);
        tcp->flags |= UV_HANDLE_SHUT;
      }
      tcp->flags &= ~(UV_HANDLE_READING | UV_HANDLE_LISTENING);
      closesocket(tcp->socket);
      if (tcp->reqs_pending == 0) {
        uv_want_endgame(handle);
      }
      return;

    case UV_NAMED_PIPE:
      pipe = (uv_pipe_t*)handle;
      pipe->flags &= ~(UV_HANDLE_READING | UV_HANDLE_LISTENING);
      close_pipe(pipe, NULL, NULL);
      if (pipe->reqs_pending == 0) {
        uv_want_endgame(handle);
      }
      return;

    case UV_TIMER:
      uv_timer_stop((uv_timer_t*)handle);
      uv_want_endgame(handle);
      return;

    case UV_PREPARE:
      uv_prepare_stop((uv_prepare_t*)handle);
      uv_want_endgame(handle);
      return;

    case UV_CHECK:
      uv_check_stop((uv_check_t*)handle);
      uv_want_endgame(handle);
      return;

    case UV_IDLE:
      uv_idle_stop((uv_idle_t*)handle);
      uv_want_endgame(handle);
      return;

    case UV_ASYNC:
      if (!((uv_async_t*)handle)->async_sent) {
        uv_want_endgame(handle);
      }
      return;

    case UV_PROCESS:
      process = (uv_process_t*)handle;
      uv_process_close(process);
      return;

    default:
      /* Not supported */
      abort();
  }
}


void uv_close(uv_handle_t* handle, uv_close_cb close_cb) {
  handle->close_cb = close_cb;
  uv_close_error(handle, uv_ok_);
}


void uv_want_endgame(uv_handle_t* handle) {
  if (!(handle->flags & UV_HANDLE_ENDGAME_QUEUED)) {
    handle->flags |= UV_HANDLE_ENDGAME_QUEUED;

    handle->endgame_next = LOOP->endgame_handles;
    LOOP->endgame_handles = handle;
  }
}


void uv_process_endgames() {
  uv_handle_t* handle;

  while (LOOP->endgame_handles) {
    handle = LOOP->endgame_handles;
    LOOP->endgame_handles = handle->endgame_next;

    handle->flags &= ~UV_HANDLE_ENDGAME_QUEUED;

    switch (handle->type) {
      case UV_TCP:
        uv_tcp_endgame((uv_tcp_t*)handle);
        break;

      case UV_NAMED_PIPE:
        uv_pipe_endgame((uv_pipe_t*)handle);
        break;

      case UV_TIMER:
        uv_timer_endgame((uv_timer_t*)handle);
        break;

      case UV_PREPARE:
      case UV_CHECK:
      case UV_IDLE:
        uv_loop_watcher_endgame(handle);
        break;

      case UV_ASYNC:
        uv_async_endgame((uv_async_t*)handle);
        break;

      case UV_PROCESS:
        uv_process_endgame((uv_process_t*)handle);
        break;

      default:
        assert(0);
        break;
    }
  }
}
