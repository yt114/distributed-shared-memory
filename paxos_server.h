//
// Created by ye on 11/22/20.
//

#ifndef DSM_PAXOS_SERVER_H
#define DSM_PAXOS_SERVER_H

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "paxos.grpc.pb.h"
#include <unordered_map>
#include <mutex>
#include <deque>
#include <queue>
#include <grpc/support/log.h>
#include <sstream>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerCompletionQueue;
using grpc::Channel;
using grpc::ClientContext;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;

using namespace  std;


class Proposer final : PaxosProposer::Service{
public:
    Status client_request(::grpc::ServerContext* context,
                         const ::ClientRequestPacket* request,
                         ::ServerResponsePacket* response) override;

private:
    mutex request_lock;
    unordered_map<string, string> memory_cache;
    unordered_map<int, int> clientID_2_latest_command;
};


class Acceptor final : PaxosAccepter::Service{
public:
    Status Prepare(::grpc::ServerContext* context, const ::PreparePacket* request, ::PromisePacket* response) override;

    Status Propose(::grpc::ServerContext* context, const ::ProposePacket* request, ::AcceptPacket* response) override;
};

#endif //DSM_PAXOS_SERVER_H
