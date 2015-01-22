/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "src/core/iomgr/pollset_kick_eventfd.h"

#ifdef GPR_LINUX_EVENTFD
#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <grpc/support/port_platform.h>
#include <grpc/support/log.h>

static void eventfd_create(grpc_kick_fd_info *fd_info) {
  int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  /* TODO(klempner): Handle failure more gracefully */
  GPR_ASSERT(efd >= 0);
  fd_info->read_fd = efd;
  fd_info->write_fd = -1;
}

static void eventfd_consume(grpc_kick_fd_info *fd_info) {
  eventfd_t value;
  int err;
  do {
    err = eventfd_read(fd_info->read_fd, &value);
  } while (err < 0 && errno == EINTR);
}

static void eventfd_kick(grpc_kick_fd_info *fd_info) {
  int err;
  do {
    err = eventfd_write(fd_info->read_fd, 1);
  } while (err < 0 && errno == EINTR);
}

static void eventfd_destroy(grpc_kick_fd_info *fd_info) {
  close(fd_info->read_fd);
}

static const grpc_pollset_kick_vtable eventfd_kick_vtable = {
  eventfd_create, eventfd_consume, eventfd_kick, eventfd_destroy
};

const grpc_pollset_kick_vtable *grpc_pollset_kick_eventfd_init(void) {
  /* TODO(klempner): Check that eventfd works */
  return &eventfd_kick_vtable;
}

#else  /* GPR_LINUX_EVENTFD not defined */
const grpc_pollset_kick_vtable *grpc_pollset_kick_eventfd_init(void) {
  return NULL;
}

#endif /* GPR_LINUX_EVENTFD */
