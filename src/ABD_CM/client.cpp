//
// Created by ye on 10/25/20.
//

#include "client.h"

static const std::chrono::high_resolution_clock::time_point beginTime = high_resolution_clock::now();
static string null_value = "00000";
static int null_lt = -1;
/* query is blocked until a majorty server replies. Replies included failure case.
 */
int query(const struct Client* c, string& key, vector<reply_holder>& replies, int time_wait, stringstream& ss){
    CompletionQueue cq;
    vector<shared_ptr<request_holder>> holders;
    int reply_server_num = 0;
    int sucess_server_reply_num = 0;
    int server_num = c->number_of_servers;
    //asychronisedly sent Query packet to all servers
    for (int i=0; i<server_num; i++) {
        ClientContext context;
        QueryRequestPacket request_packet;

        long tag = i; //tag for completion queue
        auto status = new Status;
        auto reply_packet = new ReplyPacket;

        string server_addr(c->servers[i].ip);
        server_addr += ":" + to_string(c->servers[i].port);

        cout<<"server addr:" << server_addr<<endl;
        shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                                  grpc::InsecureChannelCredentials());
        unique_ptr<LinearReadWrite::Stub> stub_ = LinearReadWrite::NewStub(toserverChannel);

        request_packet.set_clientid(c->id);//current client ID for debug
        request_packet.set_key(key);

//        chrono::system_clock::time_point deadline =
//                std::chrono::system_clock::now() + std::chrono::milliseconds(time_wait);
//        context.set_deadline(deadline);
        unique_ptr<ClientAsyncResponseReader<ReplyPacket>> rpc(
                stub_->PrepareAsyncquery(&context, request_packet, &cq));

        rpc->StartCall();
        rpc->Finish(reply_packet, status, (void *) tag);

        //move vars to request_holder
        shared_ptr<request_holder> cur_request(new request_holder);
        cur_request->serverChannel = toserverChannel;
        cur_request->stub_ = move(stub_);
        cur_request->rpc = move(rpc);
        cur_request->tag = tag;
        cur_request->status = status;
        cur_request->reply_packet = reply_packet;

        holders.push_back(cur_request);
    }

    cout<<"finsh query"<<endl;

    // break when a majority server reply arrive or
    // sum reply server number and failed server equals all server num
    while ((sucess_server_reply_num <  server_num / 2 + 1 )&&(reply_server_num < server_num)) {
        void *got_tag;
        bool ok = false;

        GPR_ASSERT(cq.Next(&got_tag, &ok));
        GPR_ASSERT(ok);

        long tag_num = (long)got_tag;


        ss<<"current thread id "<<this_thread::get_id();
        ss<<" query get tag: "<<tag_num<<" for key: "<<key<<endl;

        auto cur_request = holders[tag_num];

        if (cur_request->status->ok()) {
            ss<<" reply serverid: " << cur_request->reply_packet->serverid();
            ss<<" reply value: " << cur_request->reply_packet->value();
            sucess_server_reply_num ++;
            reply_holder cur_reply;

            cur_reply.serverID = cur_request->reply_packet->serverid();
            cur_reply.lt = cur_request->reply_packet->lt();
            cur_reply.clientID = cur_request->reply_packet->clientid();
            cur_reply.value = cur_request->reply_packet->value();
            cur_reply.is_success = cur_request->reply_packet->is_success();
            replies.push_back(cur_reply);
            ss<< " lt; ("<<cur_reply.lt<<","<<cur_reply.clientID<<") value: " << cur_reply.value<<endl;
        } else {
            cout<<"-----------";
            cout<<cur_request->status->error_message();
            cout<<"--------------"<<endl;
            ss << " RPC failed " << cur_request->status->error_message() << endl;
        }
        reply_server_num += 1;
    }

    //error checking
    if (sucess_server_reply_num < server_num / 2 + 1) return -1;
    else return 0;
}

int update(const struct Client* c, string& key, string& value, int lt, int clientID,
           vector<reply_holder>& replies, int time_wait, stringstream& ss){
    CompletionQueue cq;
    vector<shared_ptr<request_holder>> holders;
    int reply_server_num = 0;
    int sucess_server_reply_num = 0;
    int server_num = c->number_of_servers;
    int cur_clientID = c->id;

    //asychronisedly sent Query packet to all servers
    for (int i=0; i<server_num; i++) {
        ClientContext context;
        UpdateRequestPacket request_packet;

        long tag = i; //tag for completion queue
        auto status = new Status;
        auto reply_packet = new ReplyPacket;

        string server_addr(c->servers[i].ip);
        server_addr += ":" + to_string(c->servers[i].port);
        cout<<server_addr<<endl;
        shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                                  grpc::InsecureChannelCredentials());
        unique_ptr<LinearReadWrite::Stub> stub_ = LinearReadWrite::NewStub(toserverChannel);

        request_packet.set_lt(lt);
        request_packet.set_clientid(clientID);// TIMESTAMP (LT, CLIENT ID)
        request_packet.set_key(key);
        request_packet.set_value(value);
        request_packet.set_fromclient(cur_clientID);


        chrono::system_clock::time_point deadline =
                std::chrono::system_clock::now() + std::chrono::milliseconds(time_wait);
        context.set_deadline(deadline);
        unique_ptr<ClientAsyncResponseReader<ReplyPacket>> rpc(
                stub_->PrepareAsyncupdate(&context, request_packet, &cq));

        rpc->StartCall();
        rpc->Finish(reply_packet, status, (void *) tag);

        //move vars to request_holder
        shared_ptr<request_holder> cur_request(new request_holder);
        cur_request->serverChannel = toserverChannel;
        cur_request->stub_ = move(stub_);
        cur_request->rpc = move(rpc);
        cur_request->tag = tag;
        cur_request->status = status;
        cur_request->reply_packet = reply_packet;

        holders.push_back(cur_request);
    }

    // break when a majority server reply arrive or
    // sum reply server number and failed server equals all server num
    while ((sucess_server_reply_num <  server_num / 2 + 1 )&&(reply_server_num < server_num)) {
        void *got_tag;
        bool ok = false;

        GPR_ASSERT(cq.Next(&got_tag, &ok));
        GPR_ASSERT(ok);

        long tag_num = (long)got_tag;

        ss<<" current thread id "<<this_thread::get_id();
        ss<<" update get tag: "<<tag_num<<" for key: "<<key<<endl;

        auto cur_request = holders[tag_num];
        if (cur_request->status->ok()) {
            ss<<" reply serverid: " << cur_request->reply_packet->serverid();

            sucess_server_reply_num ++;
            reply_holder cur_reply;

            cur_reply.serverID = cur_request->reply_packet->serverid();
            cur_reply.lt = cur_request->reply_packet->lt();
            cur_reply.clientID = cur_request->reply_packet->clientid();
            cur_reply.value = cur_request->reply_packet->value();
            cur_reply.is_success = cur_request->reply_packet->is_success();
            replies.push_back(cur_reply);
            if (cur_request->reply_packet->is_success()) ss<<" updated to server "<<endl;
            else ss<<" not updated to server, server reply latest value pair"<<endl;

            ss<< " lt: ("<<cur_reply.lt<<","<<cur_reply.clientID<<") value: " << cur_reply.value<<endl;
        } else {
            ss << " RPC failed " << cur_request->status->error_message() << endl;
        }
        reply_server_num += 1;
    }

    //error checking
    if (sucess_server_reply_num < server_num / 2 + 1) return -1;
    else return 0;
}

string Linear_read(const struct Client* c, string key, int time_wait){//clientID is current client id not timestamp pair
    //query code
    int clientID = c->id;
    stringstream ss;
    ss<<"Linear read on key "<<key<<endl;
    int ret, time_lt = -1, time_clientID = -1;
    string value;
    vector<reply_holder> replies;
    ret = query(c, key, replies, time_wait, ss);
    if (ret < 0) {
        ss<<" thread id "<< this_thread::get_id() << "  fails at linear_read query" << endl;
    }

    for (auto& packet : replies){
        if (packet.lt > time_lt || (packet.lt == time_lt && packet.clientID > time_clientID)) {
            time_lt = packet.lt;
            time_clientID = packet.clientID;
            value = packet.value;
        }
    }

    //update
    if (value != null_value) {
        vector<reply_holder> update_replies;
        ret = update(c, key, value, time_lt, time_clientID, update_replies, time_wait,  ss);
        if (ret < 0) {
            ss<< " thread id " << this_thread::get_id() << "  fails at linear_read update" << endl;
        }
    }

    ss<<" linear read finishes, get value "<<value<<endl;
    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;
    return value;
}

int Linear_write(const struct Client* c, string key, string value_to_update, int time_wait){//clientID is current client id not timestamp pair
    stringstream ss;
    ss<<"Linear write  key: "<<key<< " value: "<<value_to_update<<endl;
    //query code
    int ret, time_lt = -1, time_clientID = -1;
    string old_value;
    vector<reply_holder> replies;
    ret = query(c, key, replies, time_wait,  ss);
    if (ret < 0) {
        cout<<" thread id "<< this_thread::get_id() << "  fails at linear_write_query" << endl;
    }

    for (auto& packet : replies){
        if (packet.lt > time_lt || (packet.lt == time_lt && packet.clientID > time_clientID)) {
            time_lt = packet.lt;
            time_clientID = packet.clientID;
            old_value = packet.value;
        }
    }
    ss<<" get old time ("<<time_lt<<", "<<time_clientID<<")" << " value: "<< old_value;
    //update
    vector<reply_holder> update_replies;
    ret = update(c, key, value_to_update, time_lt+1, c->id, update_replies, time_wait, ss);
    if (ret < 0) {
        cout<<" thread id "<< this_thread::get_id() << "  fails at linear_write update" << endl;
    }
    ss<<" linear write finishes"<<endl;
    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;
    return ret;
}

string cm_read(const struct Client* c, string& key){
    stringstream ss;
    ss<<"Linear read on key "<<key<<endl;

    int local_process_id = c->id;
    string server_addr(c->servers[local_process_id].ip);
    server_addr += ":" + to_string(c->servers[local_process_id].port);

    cout<<server_addr<<endl;
    shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                              grpc::InsecureChannelCredentials());
    unique_ptr<CMReadWrite::Stub> stub_ = CMReadWrite::NewStub(toserverChannel);

    CMClientRequestPacket request; CMClientReplyPacket reply; ClientContext context;
    request.set_is_write(false);
    request.set_key(key);
    request.set_value(null_value);

    Status status = stub_->cm_client_request(&context, request, &reply);
    if (status.ok()){
        ss<<"get return value: "<<reply.ret_val()<<endl;
    }

    ss<<" cm read finishes"<<endl;

    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;

    return reply.ret_val();
}

string cm_write(const struct Client* c, string& key, string& value){
    stringstream ss;
    ss<<"Linear write on key "<<key<<endl;

    int local_process_id = c->id;
    string server_addr(c->servers[local_process_id].ip);
    server_addr += ":" + to_string(c->servers[local_process_id].port);

    cout<<server_addr<<endl;
    shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                              grpc::InsecureChannelCredentials());
    unique_ptr<CMReadWrite::Stub> stub_ = CMReadWrite::NewStub(toserverChannel);

    CMClientRequestPacket request; CMClientReplyPacket reply; ClientContext context;
    request.set_is_write(true);
    request.set_key(key);
    request.set_value(value);

    Status status = stub_->cm_client_request(&context, request, &reply);
    if (status.ok()){
        ss<<"get return value: "<<reply.ret_val()<<endl;
    }

    ss<<" cm write finishes"<<endl;

    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;

    return reply.ret_val();
}

struct Client* client_instance(const uint32_t id, const char* protocol,
                               const struct Server_info* servers, uint32_t number_of_servers){
    auto ret_client = new Client;
    ret_client->id = id;
    strcpy(ret_client->protocol, protocol);

    ret_client->servers = servers;
    ret_client->number_of_servers = number_of_servers;
    return ret_client;
}

int client_delete(struct Client* c){
    delete c;
    return 0;
};

int put(const struct Client* c, const char* key, uint32_t key_size, const char* value, uint32_t value_size){
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
    if (!strcmp(c->protocol, "ABD")) {
        Linear_write(c, updateKey, updateValue);
    } else {
        cm_write(c, updateKey, updateValue);
    }
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
    if (!strcmp(c->protocol, "ABD")) {
        ret_value = Linear_read(c, updateKey);
    } else{
        ret_value = cm_read(c, updateKey);
    }
    currentTime_span = duration_cast<duration<double>>( high_resolution_clock::now() - beginTime);
    logfile<< currentTime_span.count() <<" {:process "<< c->id << ", :type :ok, "
           << ":f :read,"  << " :value " <<ret_value<<"}"<< std::endl;
    *value_size = ret_value.size() + 1;
    char * c_val = new char[*value_size];
    strcpy(c_val, ret_value.c_str());
    *value = c_val;

    return 0;
}

//following are testcodes
/*
 *
int server_num = 3;
string server_addrs[] = {"localhost:50051",
                          "localhost:50052",
                          "localhost:50053"};

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

void run_cm_test(){
    string key = "key1"; string value = "v1";
    int local_id = 0;
    cm_read(key, local_id);

    cm_write(key, value, local_id);

    string key2 = "key2", value2 = "v2";
    cm_write(key2, value2, local_id);
    cm_read(key2, local_id);
}

void run_test(){
    string key = "k2";
    struct Client* abd_clt[NUMBER_OF_CLIENTS];
    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
        abd_clt[i] = new Client;
        abd_clt[i]->id = i;
    }

    // Do write operations concurrently
    std::vector<std::thread*> threads;
    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){

        // build a random value
        string value = "v" + to_string(i);
        // run the thread
        threads.push_back(new std::thread(Thread_helper::_put, abd_clt[i], key, value));
    }
    // Wait for all threads to join
    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
        threads[i]->join();
    }

    // Do get operations concurrently
    threads.clear();
    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){

        // run the thread
        threads.push_back(new std::thread(Thread_helper::_get, abd_clt[i], key));
    }
    // Wait for all threads to join
    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
        threads[i]->join();
    }
    // remmeber after using values, delete them to avoid memory leak

    // Clean up allocated memory in struct Client
    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
        delete abd_clt[i];
    }
}

int main(int argc, char** argv){
    //test code
//    vector<reply_holder> replies1;
//    string key1 = "k1";
//    query(key1, replies1, 1000, 0);
//
//    vector<reply_holder> replies2;
//    string key2 = "NULL";
//    query(key2, replies2, 1000, 1);
//
//    vector<reply_holder> replies3;
//    string value1 = "v1";
//    int lt1 = 200;
//    update(key1, value1, lt1, 2,replies3, 1000, 0);
//
//    vector<reply_holder> replies4;
//    string value2 = "v2";
//    int lt2 = 100;
//    update(key1, value2, lt2, 2,replies4, 1000, 0);
//
//    vector<reply_holder> replies5;
//    query(key1, replies5, 1000, 0);

//    string key1 = "k1";
//    Linear_read(key1, 500, 0);
//
//    Linear_read(key1, 500, 0);
//
//    Linear_write(key1, "newValue");
//
//    Linear_read(key1, 500, 0);
//
//    run_cm_test();
//
//
//    return 0;
}
 */
