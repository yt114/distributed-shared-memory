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
#include <sstream>
#include <stdint.h>
#include <fstream>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;
using namespace std;
using namespace std::chrono;

static const std::chrono::high_resolution_clock::time_point beginTime = high_resolution_clock::now();

int server_num = 3;
int NUMBER_OF_CLIENTS = 3;
string server_addrs[] = {"localhost:50051",
                          "localhost:50052",
                          "localhost:50053"};

struct Client{
    uint32_t id;
//    char protocol[4]; // ABD or CM
//    struct Server_info* servers; // An array containing the infromation to access each server
//    uint32_t number_of_servers; // Number of elements in the array servers
};

int request_counter = 0;
string null_value = "00000";
int null_lt = -1;

class request_holder{
public:
    ~request_holder(){
        delete status;
        delete reply_packet;
    }
    shared_ptr<Channel> serverChannel;
    unique_ptr<LinearReadWrite::Stub> stub_;
    unique_ptr<ClientAsyncResponseReader<ReplyPacket>> rpc;

    long tag;
    Status* status;
    ReplyPacket* reply_packet;
};

struct reply_holder{
    int serverID;
    int lt;
    int clientID;
    std::string value ;
    bool is_success;
};


string Linear_read(string key, int time_wait=5000, int clientID=0);

/* perform linear write
 * return int: -1 means fails, 0 means success
 * */
int Linear_write(string key, string value_to_update, int clientID, int time_wait=5000);

/*ABD primitives*/
int update(string& key, string& value, int lt, int clientID,
           vector<reply_holder>& replies, int time_wait,
           int cur_clientID, stringstream& ss);

int query(string& key, vector<reply_holder>& replies, int time_wait, int clientID, stringstream& ss);


//CM client side functions
string cm_read(string& key, int local_process_id);

/* This function will write the value specified in the variable value into the key specified by the variable key.
 * the number of characters expected in key and value are determined by key_size and value_size respectfully.
 *
 * Returns 0 on success, and -1 on error.
 */
int put(const struct Client* c, string key, string value){
    std::ofstream logfile;
    char logfilename[30];
    duration<double> currentTime_span;
    sprintf(logfilename, "log_%d.txt", c->id);
    logfile.open(logfilename, std::ofstream::out | std::ofstream::app);

    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile << "client; "<< c->id << ";" << "invoke;" << "write;" << value << ";" << currentTime_span.count() << std::endl;
    Linear_write(key, value, (int) c->id);
    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile << "client; "<< c->id << ";" << "ok;" << "write;" << value << ";" << currentTime_span.count() << std::endl;
    return 0;
}


/* This function will read the value of the key specified by the variable key.
 * the number of characters expected in key is determined by key_size. Also the read value will be written to a portion
 * of memory and the address the first element of that portion will be written to the memory pointed by the variable
 * value. And, the size of the read value will be written to the memory pointed by value_size.
 *
 * Returns 0 on success, and -1 on error.
 */
int get(const struct Client* c, string key){
    std::ofstream logfile;
    char logfilename[30];
    duration<double> currentTime_span;
    sprintf(logfilename, "log_%d.txt", c->id);
    logfile.open(logfilename, std::ofstream::out | std::ofstream::app);

    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile << c->id << ";" << "invoke;" << "read;" << "nil" << ";" << currentTime_span.count() << std::endl;
    string value = Linear_read(key, (int) c->id);
    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile << c->id << ";" << "ok;" << "read;" << value << ";" << currentTime_span.count() << std::endl;
    return 0;
}


/* This function will destroy all the memory that might have been allocated and needs to be cleaned up.
 *
 * Returns 0 on success, and -1 on error.
 */
int client_delete(struct Client* c);

namespace Thread_helper{
    void _put(const struct Client* c, string key, string value){

        int status = put(c, key, value);

        if(status == 0){ // Success
            return;
        }
        else{
            exit(-1);
        }
    }

    void _get(const struct Client* c, string key){

        int status = get(c, key);

        if(status == 0){ // Success
            return;
        }
        else{
            exit(-1);
        }
    }
}



#endif //DISTRIBUTED_SHARED_MEMORY_CLIENT_H
