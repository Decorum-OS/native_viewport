// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_FRAMEBUFFER_FRAMEBUFFER_IMPL_H_
#define APPS_FRAMEBUFFER_FRAMEBUFFER_IMPL_H_

#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/services/framebuffer/interfaces/framebuffer.mojom.h"

namespace framebuffer {

class FramebufferImpl : public mojo::Framebuffer {
 public:
  static void Create(const mojo::FramebufferProvider::CreateCallback& callback);

 protected:
  ~FramebufferImpl() override;

 private:
  FramebufferImpl(mojo::InterfaceRequest<mojo::Framebuffer> request, int fd);

  // |mojo::Framebuffer| implementation:
  void Flush(const FlushCallback& callback) override;

  mojo::StrongBinding<Framebuffer> binding_;
  int fd_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(FramebufferImpl);
};

}  // namespace framebuffer

#endif  // APPS_FRAMEBUFFER_FRAMEBUFFER_IMPL_H_
