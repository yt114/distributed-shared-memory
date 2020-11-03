//
// Created by ye on 10/25/20.
//

#ifndef DISTRIBUTED_SHARED_MEMORY_CLIENT_H
#define DISTRIBUTED_SHARED_MEMORY_CLIENT_H


#include<string>
#include<memory>
#include <grpcpp/grpcpp.h>
#include "sharedMemory.grpc.pb.h"
#include <grpc/support/log.h>
#include <unistd.h>
#include <chrono>
#include <thread>
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;

using namespace std;
int asyn_communicate(string server_addr, string key);
#endif //DISTRIBUTED_SHARED_MEMORY_CLIENT_H
