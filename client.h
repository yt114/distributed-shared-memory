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

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;
using namespace std;

int server_num = 3;
string server_addrs[] = {"localhost:50051",
                          "localhost:50052",
                          "localhost:50053"};

struct Client{
    uint32_t id;
    char protocol[4]; // ABD or CM
    struct Server_info* servers; // An array containing the infromation to access each server
    uint32_t number_of_servers; // Number of elements in the array servers
};

/* This function will instantiate a client and initialize all the necessary variables.
 * id is the id of the client. class_name can be "ABD" or "CM" indicating the type of the client.
 * servers is an array of attributes of each server. number_of_servers determine the number of elements in the array.
 *
 * Returns a pointer to the created client. It returns NULL if an error occurs.
 */
struct Client* client_instance(const uint32_t id, const char* protocol,
        const struct Server_info* servers, uint32_t number_of_servers){
    auto ret = new Client;
    ret->number_of_servers = number_of_servers;

}


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
int Linear_write(string key, string value_to_update, int time_wait=5000, int clientID=0);

/*ABD primitives*/
int update(string& key, string& value, int lt, int clientID,
           vector<reply_holder>& replies, int time_wait,
           int cur_clientID, stringstream& ss);

int query(string& key, vector<reply_holder>& replies, int time_wait, int clientID, stringstream& ss);
#endif //DISTRIBUTED_SHARED_MEMORY_CLIENT_H
