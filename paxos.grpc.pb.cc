// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: paxos.proto

#include "paxos.pb.h"
#include "paxos.grpc.pb.h"

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

static const char* PaxosProposer_method_names[] = {
  "/PaxosProposer/client_request",
};

std::unique_ptr< PaxosProposer::Stub> PaxosProposer::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< PaxosProposer::Stub> stub(new PaxosProposer::Stub(channel));
  return stub;
}

PaxosProposer::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_client_request_(PaxosProposer_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status PaxosProposer::Stub::client_request(::grpc::ClientContext* context, const ::ClientRequestPacket& request, ::ServerResponsePacket* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_client_request_, context, request, response);
}

void PaxosProposer::Stub::experimental_async::client_request(::grpc::ClientContext* context, const ::ClientRequestPacket* request, ::ServerResponsePacket* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_client_request_, context, request, response, std::move(f));
}

void PaxosProposer::Stub::experimental_async::client_request(::grpc::ClientContext* context, const ::ClientRequestPacket* request, ::ServerResponsePacket* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_client_request_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::ServerResponsePacket>* PaxosProposer::Stub::PrepareAsyncclient_requestRaw(::grpc::ClientContext* context, const ::ClientRequestPacket& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::ServerResponsePacket>::Create(channel_.get(), cq, rpcmethod_client_request_, context, request, false);
}

::grpc::ClientAsyncResponseReader< ::ServerResponsePacket>* PaxosProposer::Stub::Asyncclient_requestRaw(::grpc::ClientContext* context, const ::ClientRequestPacket& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncclient_requestRaw(context, request, cq);
  result->StartCall();
  return result;
}

PaxosProposer::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      PaxosProposer_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< PaxosProposer::Service, ::ClientRequestPacket, ::ServerResponsePacket>(
          [](PaxosProposer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::ClientRequestPacket* req,
             ::ServerResponsePacket* resp) {
               return service->client_request(ctx, req, resp);
             }, this)));
}

PaxosProposer::Service::~Service() {
}

::grpc::Status PaxosProposer::Service::client_request(::grpc::ServerContext* context, const ::ClientRequestPacket* request, ::ServerResponsePacket* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


static const char* PaxosAccepter_method_names[] = {
  "/PaxosAccepter/Prepare",
  "/PaxosAccepter/Propose",
};

std::unique_ptr< PaxosAccepter::Stub> PaxosAccepter::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< PaxosAccepter::Stub> stub(new PaxosAccepter::Stub(channel));
  return stub;
}

PaxosAccepter::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_Prepare_(PaxosAccepter_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Propose_(PaxosAccepter_method_names[1], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status PaxosAccepter::Stub::Prepare(::grpc::ClientContext* context, const ::PreparePacket& request, ::PromisePacket* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_Prepare_, context, request, response);
}

void PaxosAccepter::Stub::experimental_async::Prepare(::grpc::ClientContext* context, const ::PreparePacket* request, ::PromisePacket* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_Prepare_, context, request, response, std::move(f));
}

void PaxosAccepter::Stub::experimental_async::Prepare(::grpc::ClientContext* context, const ::PreparePacket* request, ::PromisePacket* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_Prepare_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::PromisePacket>* PaxosAccepter::Stub::PrepareAsyncPrepareRaw(::grpc::ClientContext* context, const ::PreparePacket& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::PromisePacket>::Create(channel_.get(), cq, rpcmethod_Prepare_, context, request, false);
}

::grpc::ClientAsyncResponseReader< ::PromisePacket>* PaxosAccepter::Stub::AsyncPrepareRaw(::grpc::ClientContext* context, const ::PreparePacket& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncPrepareRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status PaxosAccepter::Stub::Propose(::grpc::ClientContext* context, const ::ProposePacket& request, ::AcceptPacket* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_Propose_, context, request, response);
}

void PaxosAccepter::Stub::experimental_async::Propose(::grpc::ClientContext* context, const ::ProposePacket* request, ::AcceptPacket* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_Propose_, context, request, response, std::move(f));
}

void PaxosAccepter::Stub::experimental_async::Propose(::grpc::ClientContext* context, const ::ProposePacket* request, ::AcceptPacket* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_Propose_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::AcceptPacket>* PaxosAccepter::Stub::PrepareAsyncProposeRaw(::grpc::ClientContext* context, const ::ProposePacket& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::AcceptPacket>::Create(channel_.get(), cq, rpcmethod_Propose_, context, request, false);
}

::grpc::ClientAsyncResponseReader< ::AcceptPacket>* PaxosAccepter::Stub::AsyncProposeRaw(::grpc::ClientContext* context, const ::ProposePacket& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncProposeRaw(context, request, cq);
  result->StartCall();
  return result;
}

PaxosAccepter::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      PaxosAccepter_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< PaxosAccepter::Service, ::PreparePacket, ::PromisePacket>(
          [](PaxosAccepter::Service* service,
             ::grpc::ServerContext* ctx,
             const ::PreparePacket* req,
             ::PromisePacket* resp) {
               return service->Prepare(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      PaxosAccepter_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< PaxosAccepter::Service, ::ProposePacket, ::AcceptPacket>(
          [](PaxosAccepter::Service* service,
             ::grpc::ServerContext* ctx,
             const ::ProposePacket* req,
             ::AcceptPacket* resp) {
               return service->Propose(ctx, req, resp);
             }, this)));
}

PaxosAccepter::Service::~Service() {
}

::grpc::Status PaxosAccepter::Service::Prepare(::grpc::ServerContext* context, const ::PreparePacket* request, ::PromisePacket* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status PaxosAccepter::Service::Propose(::grpc::ServerContext* context, const ::ProposePacket* request, ::AcceptPacket* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


