//
// Created by ye on 10/25/20.
//

#include "client.h"

string server_addrs[3] = {"localhost:50051",
                          "localhost:50052",
                          "localhost:50053"};

int request_counter = 0;
int server_num = 3;

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


//int asyn_communicate(string key, int time_wait=1000){
//    CompletionQueue cq;
//    vector<shared_ptr<request_holder>> holders;
//    int reply_server_num = 0;
//
//    for (int i=0; i<server_num; i++) {
//        ClientContext context;
//        request req;
//        long tag = i;
//
//        auto reply = new ack;
//        auto status = new Status;
//        shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addrs[i],
//                                                                  grpc::InsecureChannelCredentials());
//        unique_ptr<LinearReadWrite::Stub> stub_ = LinearReadWrite::NewStub(toserverChannel);
//
//        req.set_clientid(0);
//        req.set_key(key);
//        req.set_lt(request_counter);
//
////        chrono::system_clock::time_point deadline =
////                std::chrono::system_clock::now() + std::chrono::milliseconds(time_wait);
////        context.set_deadline(deadline);
//        unique_ptr<ClientAsyncResponseReader<ack>> rpc(
//                stub_->PrepareAsynclinear_read(&context, req, &cq));
//
//        rpc->StartCall();
//        rpc->Finish(reply, status, (void *) tag);
//
//        //move vars to request_holder
//        shared_ptr<request_holder> cur_request(new request_holder);
//        cur_request->serverChannel = toserverChannel;
//        cur_request->stub_ = move(stub_);
//        cur_request->rpc = move(rpc);
//        cur_request->tag = tag;
//        cur_request->status = status;
//        cur_request->reply_packet = rep
//
//        holders.push_back(cur_request);
//    }
//
//    while (reply_server_num <  1 ) {
//        void *got_tag;
//        bool ok = false;
//
//        GPR_ASSERT(cq.Next(&got_tag, &ok));
//        GPR_ASSERT(ok);
//
//        long tag_num = (long)got_tag;
//        printf("get tag: %li\n", tag_num);
//
//        auto cur_request = holders[tag_num];
//        if (cur_request->status->ok()) {
//            cout << "reply serverid:" << cur_request->reply->serverid() << "reply lt (request counter): " << cur_request->reply->lt()<< endl;
//            reply_server_num += 1;
//        } else {
//            cout << "RPC failed " << cur_request->status->error_message() << endl;
//        }
//
//    }
//
//    request_counter += 1;
//    return 0;
//}


/* query is blocked until a majorty server replies. Replies included failure case.
 */
int query(string& key, vector<reply_holder>& replies, int time_wait, int clientID){
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

        cout<<"current thread id "<<this_thread::get_id();
        printf(" query get tag: %li\n", tag_num);

        auto cur_request = holders[tag_num];
        if (cur_request->status->ok()) {
            cout << "reply serverid:" << cur_request->reply_packet->serverid();

            sucess_server_reply_num ++;
            reply_holder cur_reply;

            cur_reply.serverID = cur_request->reply_packet->serverid();
            cur_reply.lt = cur_request->reply_packet->lt();
            cur_reply.clientID = cur_request->reply_packet->clientid();
            cur_reply.value = cur_request->reply_packet->value();
            cur_reply.is_success = cur_request->reply_packet->is_success();
            replies.push_back(cur_reply);
        } else {
            cout << "RPC failed " << cur_request->status->error_message() << endl;
        }
        reply_server_num += 1;
    }

    request_counter += 1;
    //error checking
    if (sucess_server_reply_num < server_num / 2 + 1) return -1;
    else return 0;
}

int update(string& key, string& value, int lt, int clientID, vector<reply_holder>& replies, int time_wait, int cur_clientID){
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

        reply_packet->set_lt(lt);
        request_packet.set_clientid(clientID);// TIMESTAMP (LT, CLIENT ID)
        request_packet.set_key(key);
        request_packet.set_value(value);


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

        cout<<"current thread id "<<this_thread::get_id();
        printf(" update get tag: %li\n", tag_num);

        auto cur_request = holders[tag_num];
        if (cur_request->status->ok()) {
            cout << "reply serverid:" << cur_request->reply_packet->serverid();

            sucess_server_reply_num ++;
            reply_holder cur_reply;

            cur_reply.serverID = cur_request->reply_packet->serverid();
            cur_reply.lt = cur_request->reply_packet->lt();
            cur_reply.clientID = cur_request->reply_packet->clientid();
            cur_reply.value = cur_request->reply_packet->value();
            cur_reply.is_success = cur_request->reply_packet->is_success();
            replies.push_back(cur_reply);
        } else {
            cout << "RPC failed " << cur_request->status->error_message() << endl;
        }
        reply_server_num += 1;
    }

    request_counter += 1;
    //error checking
    if (sucess_server_reply_num < server_num / 2 + 1) return -1;
    else return 0;
}

int Linear_read(string key, int time_wait=5000, int clientID=0){
    //query code
    int ret, time_lt = -1, time_clientID = -1;
    string value;
    vector<reply_holder> replies;
    ret = query(key, replies, time_wait, clientID);
    if (ret < 0) {
        cout<<"thread id "<< this_thread::get_id() << "  fails at linear_read query" << endl;
    }

    for (auto& packet : replies){
        if (packet.lt > time_lt || (packet.lt == time_lt && packet.clientID > time_clientID)) {
            time_lt = packet.lt;
            time_clientID = packet.clientID;
            value = packet.value;
        }
    }

    //update
    vector<reply_holder> update_replies;
    ret = update(key, value, time_lt, time_clientID, update_replies, time_wait, clientID);
    if (ret < 0) {
        cout<<"thread id "<< this_thread::get_id() << "  fails at linear_read update" << endl;
    }
}

int Linear_write(string key, string value_to_update, int time_wait=5000, int clientID=0){
    //query code
    int ret, time_lt = -1, time_clientID = -1;
    string value;
    vector<reply_holder> replies;
    ret = query(key, replies, time_wait, clientID);
    if (ret < 0) {
        cout<<"thread id "<< this_thread::get_id() << "  fails at linear_write_query" << endl;
    }

    for (auto& packet : replies){
        if (packet.lt > time_lt || (packet.lt == time_lt && packet.clientID > time_clientID)) {
            time_lt = packet.lt;
            time_clientID = packet.clientID;
            value = packet.value;
        }
    }

    //update
    vector<reply_holder> update_replies;
    ret = update(key, value_to_update, time_lt+1, clientID, update_replies, time_wait, clientID);
    if (ret < 0) {
        cout<<"thread id "<< this_thread::get_id() << "  fails at linear_write update" << endl;
    }
}

int main(){
    asyn_communicate("key1", 5000);
    asyn_communicate("key2", 5000);
    return 0;
}
