// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: sharedMemory.proto

#include "sharedMemory.pb.h"
#include "sharedMemory.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>

static const char* LinearReadWrite_method_names[] = {
  "/LinearReadWrite/query",
  "/LinearReadWrite/update",
};

std::unique_ptr< LinearReadWrite::Stub> LinearReadWrite::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< LinearReadWrite::Stub> stub(new LinearReadWrite::Stub(channel));
  return stub;
}

LinearReadWrite::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_query_(LinearReadWrite_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_update_(LinearReadWrite_method_names[1], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status LinearReadWrite::Stub::query(::grpc::ClientContext* context, const ::QueryRequestPacket& request, ::ReplyPacket* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_query_, context, request, response);
}

void LinearReadWrite::Stub::experimental_async::query(::grpc::ClientContext* context, const ::QueryRequestPacket* request, ::ReplyPacket* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_query_, context, request, response, std::move(f));
}

void LinearReadWrite::Stub::experimental_async::query(::grpc::ClientContext* context, const ::QueryRequestPacket* request, ::ReplyPacket* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_query_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::ReplyPacket>* LinearReadWrite::Stub::PrepareAsyncqueryRaw(::grpc::ClientContext* context, const ::QueryRequestPacket& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::ReplyPacket>::Create(channel_.get(), cq, rpcmethod_query_, context, request, false);
}

::grpc::ClientAsyncResponseReader< ::ReplyPacket>* LinearReadWrite::Stub::AsyncqueryRaw(::grpc::ClientContext* context, const ::QueryRequestPacket& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncqueryRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status LinearReadWrite::Stub::update(::grpc::ClientContext* context, const ::UpdateRequestPacket& request, ::ReplyPacket* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_update_, context, request, response);
}

void LinearReadWrite::Stub::experimental_async::update(::grpc::ClientContext* context, const ::UpdateRequestPacket* request, ::ReplyPacket* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_update_, context, request, response, std::move(f));
}

void LinearReadWrite::Stub::experimental_async::update(::grpc::ClientContext* context, const ::UpdateRequestPacket* request, ::ReplyPacket* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_update_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::ReplyPacket>* LinearReadWrite::Stub::PrepareAsyncupdateRaw(::grpc::ClientContext* context, const ::UpdateRequestPacket& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::ReplyPacket>::Create(channel_.get(), cq, rpcmethod_update_, context, request, false);
}

::grpc::ClientAsyncResponseReader< ::ReplyPacket>* LinearReadWrite::Stub::AsyncupdateRaw(::grpc::ClientContext* context, const ::UpdateRequestPacket& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncupdateRaw(context, request, cq);
  result->StartCall();
  return result;
}

LinearReadWrite::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      LinearReadWrite_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< LinearReadWrite::Service, ::QueryRequestPacket, ::ReplyPacket>(
          [](LinearReadWrite::Service* service,
             ::grpc::ServerContext* ctx,
             const ::QueryRequestPacket* req,
             ::ReplyPacket* resp) {
               return service->query(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      LinearReadWrite_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< LinearReadWrite::Service, ::UpdateRequestPacket, ::ReplyPacket>(
          [](LinearReadWrite::Service* service,
             ::grpc::ServerContext* ctx,
             const ::UpdateRequestPacket* req,
             ::ReplyPacket* resp) {
               return service->update(ctx, req, resp);
             }, this)));
}

LinearReadWrite::Service::~Service() {
}

::grpc::Status LinearReadWrite::Service::query(::grpc::ServerContext* context, const ::QueryRequestPacket* request, ::ReplyPacket* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status LinearReadWrite::Service::update(::grpc::ServerContext* context, const ::UpdateRequestPacket* request, ::ReplyPacket* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


static const char* CMReadWrite_method_names[] = {
  "/CMReadWrite/cm_client_request",
  "/CMReadWrite/cm_update",
};

std::unique_ptr< CMReadWrite::Stub> CMReadWrite::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< CMReadWrite::Stub> stub(new CMReadWrite::Stub(channel));
  return stub;
}

CMReadWrite::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_cm_client_request_(CMReadWrite_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_cm_update_(CMReadWrite_method_names[1], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status CMReadWrite::Stub::cm_client_request(::grpc::ClientContext* context, const ::CMClientRequestPacket& request, ::CMClientReplyPacket* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_cm_client_request_, context, request, response);
}

void CMReadWrite::Stub::experimental_async::cm_client_request(::grpc::ClientContext* context, const ::CMClientRequestPacket* request, ::CMClientReplyPacket* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_cm_client_request_, context, request, response, std::move(f));
}

void CMReadWrite::Stub::experimental_async::cm_client_request(::grpc::ClientContext* context, const ::CMClientRequestPacket* request, ::CMClientReplyPacket* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_cm_client_request_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::CMClientReplyPacket>* CMReadWrite::Stub::PrepareAsynccm_client_requestRaw(::grpc::ClientContext* context, const ::CMClientRequestPacket& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::CMClientReplyPacket>::Create(channel_.get(), cq, rpcmethod_cm_client_request_, context, request, false);
}

::grpc::ClientAsyncResponseReader< ::CMClientReplyPacket>* CMReadWrite::Stub::Asynccm_client_requestRaw(::grpc::ClientContext* context, const ::CMClientRequestPacket& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsynccm_client_requestRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status CMReadWrite::Stub::cm_update(::grpc::ClientContext* context, const ::CMUpdatePacket& request, ::CMack* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_cm_update_, context, request, response);
}

void CMReadWrite::Stub::experimental_async::cm_update(::grpc::ClientContext* context, const ::CMUpdatePacket* request, ::CMack* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_cm_update_, context, request, response, std::move(f));
}

void CMReadWrite::Stub::experimental_async::cm_update(::grpc::ClientContext* context, const ::CMUpdatePacket* request, ::CMack* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_cm_update_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::CMack>* CMReadWrite::Stub::PrepareAsynccm_updateRaw(::grpc::ClientContext* context, const ::CMUpdatePacket& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::CMack>::Create(channel_.get(), cq, rpcmethod_cm_update_, context, request, false);
}

::grpc::ClientAsyncResponseReader< ::CMack>* CMReadWrite::Stub::Asynccm_updateRaw(::grpc::ClientContext* context, const ::CMUpdatePacket& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsynccm_updateRaw(context, request, cq);
  result->StartCall();
  return result;
}

CMReadWrite::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CMReadWrite_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CMReadWrite::Service, ::CMClientRequestPacket, ::CMClientReplyPacket>(
          [](CMReadWrite::Service* service,
             ::grpc::ServerContext* ctx,
             const ::CMClientRequestPacket* req,
             ::CMClientReplyPacket* resp) {
               return service->cm_client_request(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CMReadWrite_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CMReadWrite::Service, ::CMUpdatePacket, ::CMack>(
          [](CMReadWrite::Service* service,
             ::grpc::ServerContext* ctx,
             const ::CMUpdatePacket* req,
             ::CMack* resp) {
               return service->cm_update(ctx, req, resp);
             }, this)));
}

CMReadWrite::Service::~Service() {
}

::grpc::Status CMReadWrite::Service::cm_client_request(::grpc::ServerContext* context, const ::CMClientRequestPacket* request, ::CMClientReplyPacket* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status CMReadWrite::Service::cm_update(::grpc::ServerContext* context, const ::CMUpdatePacket* request, ::CMack* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


