// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "apps/native_viewport/framebuffer_impl.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_impl_base.h"
#include "mojo/public/cpp/application/run_application.h"
#include "mojo/public/cpp/application/service_provider_impl.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "mojo/public/cpp/bindings/interface_request.h"
#include "mojo/public/cpp/system/macros.h"

namespace framebuffer {

class App : public mojo::ApplicationImplBase, public mojo::FramebufferProvider {
 public:
  App() {}
  ~App() override {}

  // |mojo::ApplicationImplBase| implementation:
  bool OnAcceptConnection(
      mojo::ServiceProviderImpl* service_provider_impl) override {
    service_provider_impl->AddService<mojo::FramebufferProvider>(
        [this](const mojo::ConnectionContext& connection_context,
               mojo::InterfaceRequest<mojo::FramebufferProvider> request) {
          binding_set_.AddBinding(this, std::move(request));
        });
    return true;
  }

  // |mojo::FramebufferProvider| implementation:
  void Create(const CreateCallback& callback) override {
    FramebufferImpl::Create(callback);
  }

 private:
  mojo::BindingSet<mojo::FramebufferProvider> binding_set_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(App);
};

}  // namespace framebuffer

MojoResult MojoMain(MojoHandle request) {
  framebuffer::App app;
  return mojo::RunApplication(request, &app);
}
