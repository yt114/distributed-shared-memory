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

struct StateMachineCommand{
    string cmd;
    string key;
    string value;

    int from_client;
    int client_cmd_id;
};

struct AccepterEntry{
    StateMachineCommand acceptedValue;
    int minProposal;
    int acceptedProposal;

    AccepterEntry(){
        minProposal = -1;
        acceptedProposal = -1;
    }
};

class Proposer final : PaxosProposer::Service{
public:
    Proposer(int id): server_id(id), min_empty_log_entry(0){}

    Status client_request(::grpc::ServerContext* context,
                         const ::ClientRequestPacket* request,
                         ::ServerResponsePacket* response) override;

    int paxos_prepare(int entry_id, int propose_num, int& acceptedProposal, StateMachineCommand& return_val);

    int paxos_propose(int entry_id, int propose_num, int& minProposal,, StateMachineCommand& propos_val);

    int execut_cmd(StateMachineCommand& cmd);
private:
    int server_id;
    int min_empty_log_entry;
    mutex propose_lock;
    unordered_map<int, int> clientID_2_latest_command;
    vector<StateMachineCommand> StateMachineLog;

    unordered_map<string, string> memory_cache;
};


class Acceptor final : PaxosAccepter::Service{
public:
    Status Prepare(::grpc::ServerContext* context, const ::PreparePacket* request, ::PromisePacket* response) override;

    Status Propose(::grpc::ServerContext* context, const ::ProposePacket* request, ::AcceptPacket* response) override;
private:
    mutex accept_lock;
    unordered_map<int, StateMachineCommand> paxos_instances;
};

#endif //DSM_PAXOS_SERVER_H
