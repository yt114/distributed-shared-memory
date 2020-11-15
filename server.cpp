//
// Created by ye on 10/26/20.
//

#include <grpcpp/create_channel.h>
#include "server.h"

Status LinearSMImpl::query(ServerContext *context, const QueryRequestPacket *request,
                           ReplyPacket *response) {
    std::lock_guard<std::mutex> lock(g_i_mutex);
    const int request_clientID = request->clientid();
    const string request_key = request->key();

    RequestValue reply_packet;
    auto it = memory_cache.find(request_key);
    if (it == memory_cache.end()){
        reply_packet.key = request_key;
        reply_packet.value = null_value;
        reply_packet.lt = null_lt;
        reply_packet.clientID = null_clientID;
    } else {
        reply_packet.key = request_key;
        reply_packet.lt = it->second.lt;
        reply_packet.clientID = it->second.clientID;
        reply_packet.value = it->second.value;
    }

    printf("server id : %i receives query from client id %i\n", ServerID, request_clientID);
    cout<<"query key: " << request_key<<endl;
    cout<<"reply packet lt:("<< reply_packet.lt << ", "<<reply_packet.clientID <<")"<<endl;
    cout<<"reply value: " << reply_packet.value<<endl;
    cout<<"**********************************************************"<<endl;
    response->set_serverid(ServerID);
    response->set_lt(reply_packet.lt);
    response->set_clientid(reply_packet.clientID);
    response->set_value(reply_packet.value);
    response->set_is_success(true); // for debug

    return Status::OK;
}

Status LinearSMImpl::update(ServerContext *context, const UpdateRequestPacket *request,
                            ReplyPacket *response) {
    std::lock_guard<std::mutex> lock(g_i_mutex);
    const int request_clientID = request->fromclient();
    const int lt_clientID = request->clientid();
    const int lt = request->lt();
    cout<<"request lt "<<lt << endl;
    const string request_key = request->key();

    response->set_is_success(false);
    RequestValue reply_packet;
    auto it = memory_cache.find(request_key);
    if (it == memory_cache.end()){
        reply_packet.key = request_key;
        reply_packet.value = request->value();
        reply_packet.lt = request->lt();
        reply_packet.clientID = lt_clientID;

        memory_cache[request_key] = reply_packet;
        response->set_is_success(true);
    } else {
        reply_packet.key = it->second.key;
        reply_packet.lt = it->second.lt;
        reply_packet.clientID = it->second.clientID;
        reply_packet.value = it->second.value;

        if ((reply_packet.lt > request->lt())||
        (reply_packet.lt==request->lt() && reply_packet.clientID >= lt_clientID )){
            response->set_is_success(false);
        } else { //update to arrived packet value
            reply_packet.lt = request->lt();
            reply_packet.clientID = lt_clientID ;
            reply_packet.value = request->value();
            response->set_is_success(true);
            memory_cache[request_key] = reply_packet;
        }
    }
    response->set_serverid(ServerID);
    response->set_lt(reply_packet.lt);
    response->set_clientid(reply_packet.clientID);
    response->set_value(reply_packet.value);

    printf("server id : %i receives update from client id %i\n", ServerID, request_clientID);
    if (response->is_success()) cout<<"update  ";
    else cout<<"not update  ";

    cout<<" key: " << request_key<<endl;
    cout<<"reply latest lt:("<< reply_packet.lt << ", "<<reply_packet.clientID <<")"<<endl;
    cout<<"reply latest value: " << reply_packet.value<<endl;
    cout<<"**********************************************************"<<endl;
    return Status::OK;
}


Status CMImpl::cm_client_request(::grpc::ServerContext *context, const ::CMClientRequestPacket *request,
                                 ::CMClientReplyPacket *response) {
    std::unique_lock<std::mutex> lock(g_i_mutex);
    if (request->is_write()){
        cout<<"start write"<<endl;
        string search_key = request->key();
        string value = request->value();

        cm_cache[search_key] = value;
        vector_time[process_id] ++;

        OutQueue.emplace_back(process_id, search_key, value, vector_time);

        response->set_ret_val(cm_cache[search_key]);
        cout <<"server process id: "<<process_id<<" get write for key: "<<search_key
             <<" new value: "<<value<<endl;
    } else { //read
        string search_key = request->key();
        auto it = cm_cache.find(search_key);
        if (it == cm_cache.end()) {
            response->set_ret_val(null_value);
        } else {
            response->set_ret_val(cm_cache[search_key]);
        }

        cout <<"server process id: "<<process_id<<" get read for key: "<<search_key
        <<" return value: "<<response->ret_val()<<endl;
    }

    response->set_is_success(true);
    return Status::OK;
}

void CMImpl::_write_to_inqueue_thread_function(int request_process_id, string key, string value, vector<int> vt){
    cout<<"try to write to inq starts"<<endl;
    std::unique_lock<std::mutex> lock(g_i_mutex);
    CMWriteTuple newpacket(request_process_id, key, value, vt);
    InQueue.push(newpacket);
    cout<<"write to inq finished"<<endl;
    cout<<"add new packet to inqueue size now is "<<InQueue.size()<<endl;
    cout<<endl;
}

Status CMImpl::cm_update(::grpc::ServerContext *context, const ::CMUpdatePacket *request, ::CMack *response) {
    string key = request->key();
    string value = request->value();
    int request_process_id = request->processid();

    int received_num_servers = request->vt_size();
    if (received_num_servers != num_server){
        cout<<"error vt has "<<received_num_servers << " server "<<" process has only know "<< num_server<<endl;
        exit(-1);
    }

    vector<int> vt(received_num_servers, -1);
    for (int i=0; i<received_num_servers; i++){
        vt[i] = request->vt(i);
    }

    thread _helper_t(&CMImpl::_write_to_inqueue_thread_function,
                     this, request_process_id,
                     key, value, vt);
    _helper_t.detach();

    cout<<"receive a packet:"<<endl;
    cout<<"process id: "<<request_process_id <<" key:" <<key;
    cout<<"value: "<<value<<endl<<"vt: ";
    for (int i=0; i<received_num_servers; i++){
        cout<<i<<" : "<< vt[i]<<" , ";
    }

    response->set_is_success(true);
    response->set_reply_processid(process_id);
    return Status::OK;
}

void send_to_server(string& server_addr, CMWriteTuple& writetuple, stringstream& ss){

    ss<<"send to server:"<<server_addr<<endl;

    shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                              grpc::InsecureChannelCredentials());
    unique_ptr<CMReadWrite::Stub> stub_ = CMReadWrite::NewStub(toserverChannel);

    CMUpdatePacket request; CMack reply; ClientContext context;
    request.set_processid(writetuple.process_id);
    request.set_key(writetuple.key);
    request.set_value(writetuple.value);

    for (int i : writetuple.message_timestamp)
        request.add_vt(i);

    Status status = stub_->cm_update(&context, request, &reply);
    if (status.ok()){
        ss<<"get return success: "<<reply.is_success()<<" from "<<reply.reply_processid()<< endl;
    } else{
        ss<<status.error_message()<<" from "<<reply.reply_processid()<< endl;
    }
}

[[noreturn]] void CMImpl::send_thread_func() {
    while (true) {
        std::unique_lock<std::mutex> lock(g_i_mutex);
        //cout << " send wake up !" << endl;
        while (!OutQueue.empty()) {
            stringstream ss;
            CMWriteTuple packet = OutQueue.front();
            OutQueue.pop_front();

            ss << " cm update on key: " << packet.key << " new value: " << packet.value << endl;
            cout << "[";
            for (int i = 0; i < packet.message_timestamp.size(); i++) {
                cout << i << " : " << packet.message_timestamp[i] << ",";
            }
            ss << "]";

            for (int i = 0; i < num_server; i++) {
                if (i != process_id) {
                    send_to_server(server_addrs[i], packet, ss);
                }
            }
            ss << "---------------------------------------------------------------------" << endl;
            cout << ss.str() << endl;
        }

        lock.unlock();
        //cout << "send put into sleep and unlock !" << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(send_deliver_wait_time));
    }

}

[[noreturn]] void CMImpl::deliver_thread_func() {
    while (true) {
        std::unique_lock<std::mutex> lock(g_i_mutex);
        //cout << "deliver send wake up !" << endl;
        bool prev_delivered = true;

        while (!InQueue.empty() && prev_delivered) {
            stringstream ss;
            CMWriteTuple packet = InQueue.top();
            int from_process = packet.process_id;
            bool cur_greater = true;
            for (int i = 0; i < vector_time.size(); i++) {
                if (i != from_process && (vector_time[i] < packet.message_timestamp[i])) {
                    cur_greater = false;
                    break;
                }
            }

            if (cur_greater && (vector_time[from_process] + 1) == packet.message_timestamp[from_process]) {
                InQueue.pop();
                vector_time[from_process]++;
                cm_cache[packet.key] = packet.value;
                ss << " cm deliver on key: " << packet.key << " new value: " << packet.value << endl;
                cout << "[";
                for (int i = 0; i < packet.message_timestamp.size(); i++) {
                    cout << i << " : " << packet.message_timestamp[i] << ",";
                }
                ss << "]";
                ss << "---------------------------------------------------------------------" << endl;
                cout << ss.str() << endl;
            } else {
                prev_delivered = false;
            }
        }

        lock.unlock();
        //cout << "deliver put into sleep and unlock !" << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(send_deliver_wait_time));
    }

}

void RunServer(const string& server_address, string& protocol, int serverid){
    ServerBuilder builder;
    if (protocol=="ABD"){
        LinearSMImpl service(server_address);
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Linear SM Server listening on " << service.server_address << std::endl;
        server->Wait();
    } else if (protocol == "CM"){
        CMImpl service(server_address, serverid, num_server);
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "CM Server listening on " << service.server_address << std::endl;
        thread send_thread(&CMImpl::send_thread_func, &service);
        thread deliver_thread(&CMImpl::deliver_thread_func, &service);
        server->Wait();
    }

}

int main(int argc, char** argv){
    ServerID = atoi(argv[1]);
    string server_addr(argv[2]);
    string protocol(argv[3]);
    //testcase
//    if (ServerID == 0) {
//        cout<<"create k1"<<endl;
//        RequestValue temp;
//        temp.value = "v1";
//        temp.clientID = 10;
//        temp.lt = 100;
//        temp.key = "k1";
//        memory_cache[temp.key] = temp;
//    }

    RunServer(server_addr, protocol, ServerID);
    return 0;
}

