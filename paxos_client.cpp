//
// Created by ye on 11/22/20.
//

#include "paxos_client.h"

static const std::chrono::high_resolution_clock::time_point beginTime = high_resolution_clock::now();
static string null_value = "00000";
static int null_lt = -1;

struct Client* client_instance(const uint32_t id, const char* protocol,
                               const struct Server_info* servers, uint32_t number_of_servers){
    auto ret_client = new Client;
    ret_client->id = id;
    strcpy(ret_client->protocol, protocol);

    ret_client->servers = servers;
    ret_client->number_of_servers = number_of_servers;
    ret_client->command_id = 0;
    return ret_client;
}

int client_delete(struct Client* c){
    delete c;
    return 0;
};


int paxos_put(struct Client* c, string key, string& value){
    stringstream ss;
    ss<<"start write on key "<<key<<endl;

    int server_id = c->id % c->number_of_servers;

    string server_addr(c->servers[server_id].ip);
    server_addr += ":" + to_string(c->servers[server_id].port);

    cout<<"send put request to server_id "<<server_id << " addr "<<server_addr<<endl;
    shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                              grpc::InsecureChannelCredentials());

    unique_ptr<PaxosProposer::Stub> stub_ = PaxosProposer::NewStub(toserverChannel);

    ClientRequestPacket request; ServerResponsePacket reply; ClientContext context;
    request.set_is_write(true);
    request.set_key(key);
    request.set_value(value);
    request.set_client_id(c->id);
    request.set_client_command_id(c->command_id);

    c->command_id ++;

    Status status = stub_->client_request(&context, request, &reply);
    if (status.ok()){
        ss<<"get return success flag value: "<<reply.is_success()<<endl;
    }

    ss<<" write finishes"<<endl;

    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;

    return reply.is_success();

}


int paxos_get(struct Client* c, string key, string& value){
    stringstream ss;
    ss<<"read write on key "<<key<<endl;

    int server_id = c->id % c->number_of_servers;

    string server_addr(c->servers[server_id].ip);
    server_addr += ":" + to_string(c->servers[server_id].port);

    cout<<"send put request to server_id "<<server_id << " addr "<<server_addr<<endl;
    shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                              grpc::InsecureChannelCredentials());

    unique_ptr<PaxosProposer::Stub> stub_ = PaxosProposer::NewStub(toserverChannel);

    ClientRequestPacket request; ServerResponsePacket reply; ClientContext context;
    request.set_is_write(false);
    request.set_key(key);
    //request.set_value(value);
    request.set_client_id(c->id);
    request.set_client_command_id(c->command_id);

    c->command_id ++;

    Status status = stub_->client_request(&context, request, &reply);
    if (status.ok()){
        ss<<"get return success flag value: "<<reply.is_success()<<endl;
        value = reply.value();
    }

    ss<<" read finishes"<<endl;

    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;

    return reply.is_success();

}

int put(struct Client* c, const char* key, uint32_t key_size, const char* value, uint32_t value_size){
    std::ofstream logfile;
    stringstream temp;

    string updateKey;
    for (int i=0; i<key_size; i++){
        updateKey += key[i];
    }

    string updateValue;
    //copy value not ended in \0
    for (int i=0; i<value_size; i++){
        updateValue += value[i];
    }

    char logfilename[30];
    duration<double> currentTime_span;
    sprintf(logfilename, "log_%d.txt", c->id);
    logfile.open(logfilename, std::ofstream::out | std::ofstream::app);

    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile<< currentTime_span.count() <<" {:process "<< c->id << ", :type :invoke, "
           << ":f :write, " << ":value " << updateValue <<"}"<<std::endl;

    paxos_put(c, updateKey, updateValue);

    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile<< currentTime_span.count() <<" {:process "<< c->id << ", :type :ok, "
           << ":f :write, " << ":value " << updateValue <<"}"<<std::endl;
    return 0;
}

int get(const struct Client* c, const char* key, uint32_t key_size, char** value, uint32_t *value_size){
    string updateKey;
    for (int i=0; i<key_size; i++){
        updateKey += key[i];
    }

    std::ofstream logfile;
    char logfilename[30];
    duration<double> currentTime_span;
    sprintf(logfilename, "log_%d.txt", c->id);
    logfile.open(logfilename, std::ofstream::out | std::ofstream::app);

    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile<< currentTime_span.count() <<" {:process "<< c->id << ", :type :invoke, "
           << ":f :read," << " :value nil}" << std::endl;
    string ret_value;

    paxos_get(c, updateKey, ret_value);
    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile<< currentTime_span.count() <<" {:process "<< c->id << ", :type :ok, "
           << ":f :read,"  << " :value " <<ret_value<<"}"<< std::endl;
    *value_size = ret_value.size() + 1;
    char * c_val = new char[*value_size];
    strcpy(c_val, ret_value.c_str());
    *value = c_val;

    return 0;
}
