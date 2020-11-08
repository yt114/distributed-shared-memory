//
// Created by ye on 10/25/20.
//

#include "client.h"


/* query is blocked until a majorty server replies. Replies included failure case.
 */
int query(string& key, vector<reply_holder>& replies, int time_wait, int clientID, stringstream& ss){
    CompletionQueue cq;
    vector<shared_ptr<request_holder>> holders;
    int reply_server_num = 0;
    int sucess_server_reply_num = 0;

    //asychronisedly sent Query packet to all servers
    for (int i=0; i<server_num; i++) {
        ClientContext context;
        QueryRequestPacket request_packet;

        long tag = i; //tag for completion queue
        auto status = new Status;
        auto reply_packet = new ReplyPacket;

        shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addrs[i],
                                                                  grpc::InsecureChannelCredentials());
        unique_ptr<LinearReadWrite::Stub> stub_ = LinearReadWrite::NewStub(toserverChannel);

        request_packet.set_clientid(clientID);//current client ID for debug
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
            ss << " RPC failed " << cur_request->status->error_message() << endl;
        }
        reply_server_num += 1;
    }
    request_counter += 1;
    //error checking
    if (sucess_server_reply_num < server_num / 2 + 1) return -1;
    else return 0;
}

int update(string& key, string& value, int lt, int clientID,
           vector<reply_holder>& replies, int time_wait,
           int cur_clientID, stringstream& ss){
    CompletionQueue cq;
    vector<shared_ptr<request_holder>> holders;
    int reply_server_num = 0;
    int sucess_server_reply_num = 0;


    //asychronisedly sent Query packet to all servers
    for (int i=0; i<server_num; i++) {
        ClientContext context;
        UpdateRequestPacket request_packet;

        long tag = i; //tag for completion queue
        auto status = new Status;
        auto reply_packet = new ReplyPacket;

        shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addrs[i],
                                                                  grpc::InsecureChannelCredentials());
        unique_ptr<LinearReadWrite::Stub> stub_ = LinearReadWrite::NewStub(toserverChannel);

        request_packet.set_lt(lt);
        request_packet.set_clientid(clientID);// TIMESTAMP (LT, CLIENT ID)
        request_packet.set_key(key);
        request_packet.set_value(value);
        request_packet.set_fromclient(cur_clientID);


//        chrono::system_clock::time_point deadline =
//                std::chrono::system_clock::now() + std::chrono::milliseconds(time_wait);
//        context.set_deadline(deadline);
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

    request_counter += 1;
    //error checking
    if (sucess_server_reply_num < server_num / 2 + 1) return -1;
    else return 0;
}

string Linear_read(string key, int time_wait, int clientID){//clientID is current client id not timestamp pair
    //query code
    stringstream ss;
    ss<<"Linear read on key "<<key<<endl;
    int ret, time_lt = -1, time_clientID = -1;
    string value;
    vector<reply_holder> replies;
    ret = query(key, replies, time_wait, clientID, ss);
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
        ret = update(key, value, time_lt, time_clientID, update_replies, time_wait, clientID, ss);
        if (ret < 0) {
            ss<< " thread id " << this_thread::get_id() << "  fails at linear_read update" << endl;
        }
    }

    ss<<" linear read finishes, get value "<<value<<endl;
    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;
    return value;
}

int Linear_write(string key, string value_to_update, int clientID, int time_wait){//clientID is current client id not timestamp pair
    stringstream ss;
    ss<<"Linear write  key: "<<key<< " value: "<<value_to_update<<endl;
    //query code
    int ret, time_lt = -1, time_clientID = -1;
    string old_value;
    vector<reply_holder> replies;
    ret = query(key, replies, time_wait, clientID, ss);
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
    ret = update(key, value_to_update, time_lt+1, clientID, update_replies, time_wait, clientID, ss);
    if (ret < 0) {
        cout<<" thread id "<< this_thread::get_id() << "  fails at linear_write update" << endl;
    }
    ss<<" linear write finishes"<<endl;
    ss<<"---------------------------------------------------------------------"<<endl;
    cout<<ss.str()<<endl;
    return ret;
}

string cm_read(string& key, int local_process_id){
    stringstream ss;
    ss<<"Linear read on key "<<key<<endl;
    shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addrs[local_process_id],
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

string cm_write(string& key, string& value, int local_process_id){
    stringstream ss;
    ss<<"Linear write on key "<<key<<endl;
    shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addrs[local_process_id],
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


void run_cm_test(){
    string key = "key1"; string value = "v1";
    int local_id = 0;
    cm_read(key, local_id);

    cm_write(key, value, local_id);
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

    run_cm_test();


    return 0;
}
