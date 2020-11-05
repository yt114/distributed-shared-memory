//
// Created by ye on 10/26/20.
//

#ifndef DISTRIBUTED_SHARED_MEMORY_SERVER_H
#define DISTRIBUTED_SHARED_MEMORY_SERVER_H
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
#include "sharedMemory.grpc.pb.h"
#include <unordered_map>
#include <mutex>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerCompletionQueue;
using namespace  std;

std::mutex g_i_mutex; // global lock for query and

int ServerID = 0;
string null_value = "00000";
int null_lt = -1;
int null_clientID = -1;
struct RequestValue{
    int clientID;
    int lt; //logic time
    string value;
    string key;
};

unordered_map<string, RequestValue> memory_cache;

class LinearSMImpl final : public LinearReadWrite::Service{
public:
    explicit LinearSMImpl(string server_addr): server_address(std::move(server_addr)){}
    Status query(ServerContext* context, const QueryRequestPacket* request, ReplyPacket* response) override;
    Status update(::grpc::ServerContext* context, const ::UpdateRequestPacket* request, ::ReplyPacket* response) override;

    string server_address;
};

void RunServer(const string& server_address);

#endif //DISTRIBUTED_SHARED_MEMORY_SERVER_H
