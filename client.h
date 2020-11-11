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
#include <cstring>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;
using namespace std;
using namespace std::chrono;

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

struct Server_info{
    char ip[16]; // IP Sample: "192.168.1.2"
    uint16_t port;
};


struct Client{
    uint32_t id;
    char protocol[4]; // ABD or CM
    const struct Server_info* servers; // An array containing the infromation to access each server
    uint32_t number_of_servers; // Number of elements in the array servers
};

string Linear_read(const struct Client* c, string key, int time_wait=5000);

/* perform linear write
 * return int: -1 means fails, 0 means success
 * */
int Linear_write(const struct Client* c, string key, string value_to_update, int time_wait=5000);

/*ABD primitives*/
int update(const struct Client* c, string& key, string& value, int lt, int clientID,
           vector<reply_holder>& replies, int time_wait, stringstream& ss);

int query(const struct Client* c, string& key, vector<reply_holder>& replies, int time_wait, stringstream& ss);


//CM client side functions
string cm_read(const struct Client* c, string& key);
string cm_write(const struct Client* c, string& value);

/*******************************************************************************/


struct Client* client_instance(const uint32_t id, const char* protocol,
        const struct Server_info* servers, uint32_t number_of_servers);


/* This function will write the value specified in the variable value into the key specified by the variable key.
 * the number of characters expected in key and value are determined by key_size and value_size respectfully.
 *
 * Returns 0 on success, and -1 on error.
 */
int put(const struct Client* c, const char* key, uint32_t key_size, const char* value, uint32_t value_size);


/* This function will read the value of the key specified by the variable key.
 * the number of characters expected in key is determined by key_size. Also the read value will be written to a portion
 * of memory and the address the first element of that portion will be written to the memory pointed by the variable
 * value. And, the size of the read value will be written to the memory pointed by value_size.
 *
 * Returns 0 on success, and -1 on error.
 */
int get(const struct Client* c, const char* key, uint32_t key_size, char** value, uint32_t *value_size);


/* This function will destroy all the memory that might have been allocated and needs to be cleaned up.
 *
 * Returns 0 on success, and -1 on error.
 */
int client_delete(struct Client* c);


#endif //DISTRIBUTED_SHARED_MEMORY_CLIENT_H
