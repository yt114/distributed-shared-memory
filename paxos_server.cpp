//
// Created by ye on 11/22/20.
//

#include "paxos_server.h"

int Proposer::paxos_prepare(int entry_id, int propose_num, int& acceptedProposal, StateMachineCommand& return_val) {

}

int Proposer::paxos_propose(int entry_id, int propose_num, int& minProposal,, StateMachineCommand& propos_val){

}

int Proposer::execut_cmd(StateMachineCommand& cmd){
    cout<<"cmd from client: "<<cmd.from_client <<" cmd_id: "<< cmd.client_cmd_id<<endl;
    if (cmd.cmd == "write"){
        cout << "write key: "<<cmd.key<<" value: "<<cmd.value;
        memory_cache[cmd.key] = cmd.value;
    } else if (cmd.cmd == "read") {
        cout << "get key: "<<cmd.key;
        if (memory_cache.find(cmd.key) == memory_cache.end()) {
            cout <<" key not find"<<endl;
            return -1;
        } else {
            cmd.value = memory_cache[cmd.key];
            cout <<" value: "<< cmd.value;
        }
    }

    return 0;
}

Status Proposer::client_request(::grpc::ServerContext *context, const ::ClientRequestPacket *request,
                                ::ServerResponsePacket *response) {
    lock_guard<mutex> lock(propose_lock);
    bool propose_success = false;
    int from_client = request->client_id();
    int command_id = request->client_command_id();
    auto it = clientID_2_latest_command.find(from_client);
    if (it == clientID_2_latest_command.end()){
        clientID_2_latest_command[from_client] = command_id;
        cout<<"server get new request from client " << from_client << " request command id " <<command_id<<endl;
    } else if (command_id <= it->second){
        cout<<"warning: client " << from_client << " request command id " <<command_id;
        cout<< "less than current command id"<<endl;
    }

    while (!propose_success){
        int entry_id = min_empty_log_entry;
        int propose_num = -1;
        bool NACK = true;

        StateMachineCommand returned_cmd;
        while(NACK) { // one census instance
            propose_num ++;
            int acceptedProposal, minProposal;
            int ret = paxos_prepare(entry_id, propose_num, acceptedProposal, returned_cmd); //census protocal phase1;

            if (ret < 0) {// phase 1 fails, increment propose num
                NACK = true;
                continue;
            }

            if (acceptedProposal < 0){//no accpeted values
                returned_cmd.from_client = from_client;
                returned_cmd.client_cmd_id = command_id;
                if (request->is_write) returned_cmd.cmd = "write";
                else returned_cmd.cmd = "read";
                returned_cmd.key = request->key();
                returned_cmd.value = request->value();
            }

            //census protocal phase 2
            ret = paxos_propose(entry_id, propose_num, returned_cmd);
            if (ret < 0){ //rejection
                NACK = true;
                continue;
            } else {
                NACK = true;
            }

        }

        cout<<"execute command: "<<endl;
        execut_cmd(returned_cmd);
        if (returned_cmd.from_client == from_client && returned_cmd.client_cmd_id = command_id){//end of census
            propose_success = true;
        } else {
            propose_success = false;
            min_empty_log_entry += 1;
        }
    }


    return Status::OK;
}