//
// Created by ye on 10/26/20.
//

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

void RunServer(const string& server_address){
    ServerBuilder builder;
    LinearSMImpl service(server_address);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << service.server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv){
    ServerID = atoi(argv[1]);
    string server_addr(argv[2]);
    //testcase
    if (ServerID == 0) {
        cout<<"create k1"<<endl;
        RequestValue temp;
        temp.value = "v1";
        temp.clientID = 10;
        temp.lt = 100;
        temp.key = "k1";
        memory_cache[temp.key] = temp;
    }

    RunServer(server_addr);
    return 0;
}