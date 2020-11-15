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
using namespace  std;

std::mutex g_i_mutex; // global lock for query and
int num_server = 3;
string server_addrs[] = {"0.0.0.0:50051", //server 0
                         "0.0.0.0:50052", //server 1
                         "0.0.0.0:50053"}; //server 2

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

struct CMWriteTuple{
    int process_id;
    string key;
    string value;
    vector<int> message_timestamp;

    CMWriteTuple(int id, string& serckey, string& updatevalue, vector<int>& vt){
        process_id = id;
        key = serckey;
        value = updatevalue;
        message_timestamp = vt;
    }
};

struct CompareWriteTuple{
public:
    bool operator() (CMWriteTuple& a, CMWriteTuple& b){// min heap return a < b
        if (a.message_timestamp.size() != b.message_timestamp.size()){
            cout<<"error: VT message size are not the same"<<endl;
            exit(-1);
        }
        bool a_greater = false, b_greater = false;
        int n = a.message_timestamp.size();
        for (int i=0; i<n; i++) {
            if (a.message_timestamp[i] != b.message_timestamp[i]) {
                if (a.message_timestamp[i] < b.message_timestamp[i]) b_greater = true;
                else a_greater = true;
            }
        }

        if (a_greater && ! b_greater) return false;
        else return true;
    }
};
unordered_map<string, string> cm_cache;
deque<CMWriteTuple> OutQueue;
priority_queue<CMWriteTuple, vector<CMWriteTuple>, CompareWriteTuple> InQueue;
int send_deliver_wait_time = 20; //20ms
class CMImpl final : public CMReadWrite::Service{
public:
    explicit CMImpl(string server_addr, int process_id, int server_num) : server_address(std::move(server_addr)),
    process_id(process_id), vector_time(server_num, 0){}

    Status cm_client_request(ServerContext* context, const CMClientRequestPacket* request,
                             CMClientReplyPacket* response) override;

    /*a non blocking function*/
    Status cm_update(::grpc::ServerContext* context, const ::CMUpdatePacket* request, ::CMack* response) override;

    /* called by cm_update. should be detached as called thread can be destory*/
    void _write_to_inqueue_thread_function(int request_process_id, string key, string value, vector<int> vt);

    [[noreturn]] void send_thread_func();

    [[noreturn]] void deliver_thread_func();

    string server_address;
    int process_id;
    vector<int> vector_time;
};
void RunServer(const string& server_address, string& protocol, int serverid=0);

#endif //DISTRIBUTED_SHARED_MEMORY_SERVER_H
