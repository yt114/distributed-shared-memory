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

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerCompletionQueue;
using namespace  std;

int32_t ServerID = 0;

class LinearSMImpl final : public LinearReadWrite::Service{
    Status linear_read(ServerContext* context,
                       const request* request,
                       ack* response) override {
        std::cout<<"read request:";
        int32_t clientID, clientLT;
        std::string key;
        clientID = request->clientid();
        clientLT = request->lt();
        key = request->key();

        cout<<"Read call client ID:"<<clientID << " clientLT:"<<clientLT<<" key:"<<key<<endl;

        this_thread::sleep_for(chrono::microseconds(200));
        //usleep(10000);
        response->set_serverid(ServerID);
        response->set_lt(clientLT);
        response->set_value("v2");
        if (context->IsCancelled()) {
            cout<<"Deadline exceeded"<<endl;
            //return Status(StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
        }
        return Status::OK;
    }
};

void RunServer(const string server_address){
    ServerBuilder builder;
    LinearSMImpl service;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}


#endif //DISTRIBUTED_SHARED_MEMORY_SERVER_H
