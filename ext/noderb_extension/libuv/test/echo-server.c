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

#include "uv.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

static int server_closed;
static stream_type serverType;
static uv_tcp_t tcpServer;
static uv_pipe_t pipeServer;
static uv_handle_t* server;

static void after_write(uv_write_t* req, int status);
static void after_read(uv_stream_t*, ssize_t nread, uv_buf_t buf);
static void on_close(uv_handle_t* peer);
static void on_server_close(uv_handle_t* handle);
static void on_connection(uv_stream_t*, int status);


static void after_write(uv_write_t* req, int status) {
  write_req_t* wr;

  if (status) {
    uv_err_t err = uv_last_error();
    fprintf(stderr, "uv_write error: %s\n", uv_strerror(err));
    ASSERT(0);
  }

  wr = (write_req_t*) req;

  /* Free the read/write buffer and the request */
  free(wr->buf.base);
  free(wr);
}


static void after_shutdown(uv_shutdown_t* req, int status) {
  uv_close((uv_handle_t*)req->handle, on_close);
  free(req);
}


static void after_read(uv_stream_t* handle, ssize_t nread, uv_buf_t buf) {
  int i;
  write_req_t *wr;
  uv_shutdown_t* req;

  if (nread < 0) {
    /* Error or EOF */
    ASSERT (uv_last_error().code == UV_EOF);

    if (buf.base) {
      free(buf.base);
    }

    req = (uv_shutdown_t*) malloc(sizeof *req);
    uv_shutdown(req, handle, after_shutdown);

    return;
  }

  if (nread == 0) {
    /* Everything OK, but nothing read. */
    free(buf.base);
    return;
  }

  /* 
   * Scan for the letter Q which signals that we should quit the server.
   * If we get QS it means close the stream.
   */
  if (!server_closed) {
    for (i = 0; i < nread; i++) {
      if (buf.base[i] == 'Q') {
        if (i + 1 < nread && buf.base[i + 1] == 'S') {
          free(buf.base);
          uv_close((uv_handle_t*)handle, NULL);
          return;
        } else {
          uv_close(server, on_server_close);
          server_closed = 1;
        }
      }
    }
  }

  wr = (write_req_t*) malloc(sizeof *wr);

  wr->buf.base = buf.base;
  wr->buf.len = nread;
  if (uv_write(&wr->req, handle, &wr->buf, 1, after_write)) {
    FATAL("uv_write failed");
  }
}


static void on_close(uv_handle_t* peer) {
  free(peer);
}


static uv_buf_t echo_alloc(uv_stream_t* handle, size_t suggested_size) {
  uv_buf_t buf;
  buf.base = (char*) malloc(suggested_size);
  buf.len = suggested_size;
  return buf;
}


static void on_connection(uv_stream_t* server, int status) {
  uv_stream_t* stream;
  int r;

  if (status != 0) {
    fprintf(stderr, "Connect error %d\n", uv_last_error().code);
  }
  ASSERT(status == 0);

  switch (serverType) {
  case TCP:
    stream = malloc(sizeof(uv_tcp_t));
    ASSERT(stream != NULL);
    uv_tcp_init((uv_tcp_t*)stream);
    break;

  case PIPE:
    stream = malloc(sizeof(uv_pipe_t));
    ASSERT(stream != NULL);
    uv_pipe_init((uv_pipe_t*)stream);
    break;

  default:
    ASSERT(0 && "Bad serverType");
    abort();
  }

  /* associate server with stream */
  stream->data = server;

  r = uv_accept(server, stream);
  ASSERT(r == 0);

  r = uv_read_start(stream, echo_alloc, after_read);
  ASSERT(r == 0);
}


static void on_server_close(uv_handle_t* handle) {
  ASSERT(handle == server);
}


static int tcp4_echo_start(int port) {
  struct sockaddr_in addr = uv_ip4_addr("0.0.0.0", port);
  int r;

  server = (uv_handle_t*)&tcpServer;
  serverType = TCP;

  r = uv_tcp_init(&tcpServer);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Socket creation error\n");
    return 1;
  }

  r = uv_tcp_bind(&tcpServer, addr);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Bind error\n");
    return 1;
  }

  r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Listen error\n");
    return 1;
  }

  return 0;
}


static int tcp6_echo_start(int port) {
  struct sockaddr_in6 addr6 = uv_ip6_addr("::1", port);
  int r;

  server = (uv_handle_t*)&tcpServer;
  serverType = TCP;

  r = uv_tcp_init(&tcpServer);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Socket creation error\n");
    return 1;
  }

  /* IPv6 is optional as not all platforms support it */
  r = uv_tcp_bind6(&tcpServer, addr6);
  if (r) {
    /* show message but return OK */
    fprintf(stderr, "IPv6 not supported\n");
    return 0;
  }

  r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection);
  if (r) {
    /* TODO: Error codes */
    fprintf(stderr, "Listen error\n");
    return 1;
  }

  return 0;
}


static int pipe_echo_start(char* pipeName) {
  int r;

  server = (uv_handle_t*)&pipeServer;
  serverType = PIPE;

  r = uv_pipe_init(&pipeServer);
  if (r) {
    fprintf(stderr, "uv_pipe_init: %s\n", uv_strerror(uv_last_error()));
    return 1;
  }

  r = uv_pipe_bind(&pipeServer, pipeName);
  if (r) {
    fprintf(stderr, "uv_pipe_bind: %s\n", uv_strerror(uv_last_error()));
    return 1;
  }

  r = uv_listen((uv_stream_t*)&pipeServer, SOMAXCONN, on_connection);
  if (r) {
    fprintf(stderr, "uv_pipe_listen: %s\n", uv_strerror(uv_last_error()));
    return 1;
  }

  return 0;
}


HELPER_IMPL(tcp4_echo_server) {
  uv_init();
  if (tcp4_echo_start(TEST_PORT))
    return 1;

  uv_run();
  return 0;
}


HELPER_IMPL(tcp6_echo_server) {
  uv_init();
  if (tcp6_echo_start(TEST_PORT))
    return 1;

  uv_run();
  return 0;
}


HELPER_IMPL(pipe_echo_server) {
  uv_init();

  if (pipe_echo_start(TEST_PIPENAME))
    return 1;

  uv_run();
  return 0;
}
