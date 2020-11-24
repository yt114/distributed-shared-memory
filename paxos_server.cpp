//
// Created by ye on 11/22/20.
//

#include "paxos_server.h"

Status Proposer::client_request(::grpc::ServerContext *context, const ::ClientRequestPacket *request,
                                ::ServerResponsePacket *response) {
    lock_guard<mutex> lock(request_lock);
    int from_client = request->client_id();
    int command_id = request->client_command_id();
    auto it = clientID_2_latest_command.find(from_client);
    if (it == clientID_2_latest_command.end()){
        clientID_2_latest_command[from_client] = command_id;
    } else if (command_id <= it->second){
        cout<<"client " << from_client << " request command id " <<command_id;
        cout<< "less than current command id"
    }




    return Status::OK;
}