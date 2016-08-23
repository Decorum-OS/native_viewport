// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/native_viewport/framebuffer_impl.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ddk/protocol/display.h>
#include <magenta/syscalls.h>
#include <mxio/io.h>

#include "lib/ftl/logging.h"

namespace framebuffer {
namespace {

constexpr char kVirtualConsole[] = "/dev/class/console/vc";

mojo::FramebufferFormat GetFramebufferFormat(int display_format) {
  switch (display_format) {
    case MX_PIXEL_FORMAT_RGB_565:
      return mojo::FramebufferFormat::RGB_565;
    case MX_PIXEL_FORMAT_RGB_332:
      return mojo::FramebufferFormat::RGB_332;
    case MX_PIXEL_FORMAT_RGB_2220:
      return mojo::FramebufferFormat::RGB_2220;
    case MX_PIXEL_FORMAT_ARGB_8888:
      return mojo::FramebufferFormat::ARGB_8888;
    case MX_PIXEL_FORMAT_RGB_x888:
      return mojo::FramebufferFormat::RGB_x888;
    case MX_PIXEL_FORMAT_MONO_1:
      return mojo::FramebufferFormat::MONO_1;
    case MX_PIXEL_FORMAT_MONO_8:
      return mojo::FramebufferFormat::MONO_8;
    default:
      return mojo::FramebufferFormat::NONE;
  }
}

}  // namespace

void FramebufferImpl::Create(
    const mojo::FramebufferProvider::CreateCallback& callback) {
  int fd = open(kVirtualConsole, O_RDWR);
  if (fd < 0) {
    FTL_DLOG(ERROR) << "Failed to open frame buffer: " << errno;
    callback.Run(nullptr, nullptr);
    return;
  }

  ioctl_display_get_fb_t frame_buffer;
  ssize_t result = mxio_ioctl(fd, DISPLAY_OP_GET_FB, nullptr, 0, &frame_buffer,
                              sizeof(frame_buffer));

  if (result < 0) {
    FTL_DLOG(ERROR) << "DISPLAY_OP_GET_FB failed.";
    close(fd);
    callback.Run(nullptr, nullptr);
    return;
  }

  mojo::ScopedHandle vmo(mojo::Handle(frame_buffer.vmo));

  mojo::FramebufferInfoPtr info = mojo::FramebufferInfo::New();
  info->format = GetFramebufferFormat(frame_buffer.info.format);
  if (info->format == mojo::FramebufferFormat::NONE) {
    FTL_DLOG(ERROR) << "Invalid framebuffer format: "
                    << frame_buffer.info.format;
    close(fd);
    callback.Run(nullptr, nullptr);
    return;
  }

  info->size = mojo::Size::New();
  info->size->width = frame_buffer.info.width;
  info->size->height = frame_buffer.info.height;
  info->row_bytes = frame_buffer.info.stride * frame_buffer.info.pixelsize;
  info->vmo = std::move(vmo);

  mojo::FramebufferPtr framebuffer;
  new FramebufferImpl(mojo::GetProxy(&framebuffer), fd);
  callback.Run(std::move(framebuffer), std::move(info));
}

FramebufferImpl::FramebufferImpl(
    mojo::InterfaceRequest<mojo::Framebuffer> request,
    int fd)
    : binding_(this, std::move(request)), fd_(fd) {}

FramebufferImpl::~FramebufferImpl() {
  close(fd_);
}

void FramebufferImpl::Flush(const FlushCallback& callback) {
  ssize_t result = mxio_ioctl(fd_, DISPLAY_OP_FLUSH_FB, nullptr, 0, nullptr, 0);
  if (result < 0) {
    FTL_DLOG(ERROR) << "DISPLAY_OP_FLUSH_FB failed.";
    binding_.Close();
    return;
  }

  callback.Run();
}

}  // namespace framebuffer
