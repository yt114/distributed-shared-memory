//
// Created by ye on 11/22/20.
//

#include "paxos_server.h"

const int accepter_nums = 3;

string accepter_addrs[] = {"localhost:50050",
                           "localhost:50051",
                           "localhost:50052"};



int Proposer::paxos_prepare(int entry_id, int propose_num, int& acceptedProposal, StateMachineCommand& return_val) {
    CompletionQueue cq;
    vector<shared_ptr<prepare_request_holder>> holders;

    int reply_server_num = 0;
    int sucess_server_reply_num = 0;
    int server_num = accepter_nums;

    for (int i=0; i<accepter_nums; i++){
        ClientContext context;
        PreparePacket request_packet;

        long tag = i; //tag for completion queue
        auto status = new Status;
        auto reply_packet = new PromisePacket;

        string server_addr(accepter_addrs[i]);
        cout<<"send to accepter addr:" << server_addr<<endl;

        shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                                  grpc::InsecureChannelCredentials());

        unique_ptr<PaxosAccepter::Stub> stub_ = PaxosAccepter::NewStub(toserverChannel);

        request_packet.set_log_entry(entry_id);
        request_packet.set_server_id(server_id);
        request_packet.set_proposal_num(propose_num);

        unique_ptr<ClientAsyncResponseReader<PromisePacket>> rpc(
                stub_->PrepareAsyncPrepare(&context, request_packet, &cq));

        rpc->StartCall();
        rpc->Finish(reply_packet, status, (void *) tag);

        //move vars to request_holder
        shared_ptr<prepare_request_holder> cur_request(new prepare_request_holder);
        cur_request->serverChannel = toserverChannel;
        cur_request->stub_ = move(stub_);
        cur_request->rpc = move(rpc);
        cur_request->tag = tag;
        cur_request->status = status;
        cur_request->reply_packet = reply_packet;

        holders.push_back(cur_request);
    }

    cout<<"finsh query"<<endl;

    while ((sucess_server_reply_num <  accepter_nums / 2 + 1 )&&(reply_server_num < accepter_nums)) {
        void *got_tag;
        bool ok = false;

        GPR_ASSERT(cq.Next(&got_tag, &ok));
        GPR_ASSERT(ok);

        long tag_num = (long)got_tag;

        auto cur_request = holders[tag_num];

        if (cur_request->status->ok()) {
            cout<<" reply accepter id: " << cur_request->reply_packet->server_id();
            if (cur_request->reply_packet->error()){
                cout<<" get NACK "<< endl;
                return -1;
            }
            sucess_server_reply_num ++;

            cout<<" reply accepted proposal num" << cur_request->reply_packet->accepted_id();

            int accepted_proposal_num = cur_request->reply_packet->max_proposal_num();

            if (accepted_proposal_num > acceptedProposal){
                acceptedProposal = accepted_proposal_num;
                //set cmd

                return_val.from_client = cur_request->reply_packet->client_id();
                return_val.client_cmd_id = cur_request->reply_packet->client_command_id();

                if (cur_request->reply_packet->is_write()) return_val.cmd = "write";
                else return_val.cmd = "read";

                return_val.key = cur_request->reply_packet->key();
                return_val.value = cur_request->reply_packet->value();
            }

        } else {
            cout<<"-----------";
            cout<<cur_request->status->error_message();
            cout<<"--------------"<<endl;
            cout << " RPC failed " << cur_request->status->error_message() << endl;
        }
        reply_server_num += 1;
    }

    return 0;
}

int Proposer::paxos_propose(int entry_id, int propose_num,  StateMachineCommand& propos_val){
    CompletionQueue cq;
    vector<shared_ptr<propose_request_holder>> holders;

    int reply_server_num = 0;
    int sucess_server_reply_num = 0;
    int server_num = accepter_nums;

    for (int i=0; i<accepter_nums; i++){
        ClientContext context;
        ProposePacket request_packet;

        long tag = i; //tag for completion queue
        auto status = new Status;
        auto reply_packet = new AcceptPacket;

        string server_addr(accepter_addrs[i]);
        cout<<"send to accepter addr:" << server_addr<<endl;

        shared_ptr<Channel> toserverChannel = grpc::CreateChannel(server_addr,
                                                                  grpc::InsecureChannelCredentials());

        unique_ptr<PaxosAccepter::Stub> stub_ = PaxosAccepter::NewStub(toserverChannel);

        request_packet.set_log_entry(entry_id);
        request_packet.set_server_id(server_id);
        request_packet.set_proposal_num(propose_num);

        request_packet.set_is_write(propos_val.cmd == "write");
        request_packet.set_key(propos_val.key);
        request_packet.set_value(propos_val.value);

        unique_ptr<ClientAsyncResponseReader<AcceptPacket>> rpc(
                stub_->PrepareAsyncPropose(&context, request_packet, &cq));

        rpc->StartCall();
        rpc->Finish(reply_packet, status, (void *) tag);

        //move vars to request_holder
        shared_ptr<propose_request_holder> cur_request(new propose_request_holder);
        cur_request->serverChannel = toserverChannel;
        cur_request->stub_ = move(stub_);
        cur_request->rpc = move(rpc);
        cur_request->tag = tag;
        cur_request->status = status;
        cur_request->reply_packet = reply_packet;

        holders.push_back(cur_request);
    }

    cout<<"finsh query"<<endl;

    while ((sucess_server_reply_num <  accepter_nums / 2 + 1 )&&(reply_server_num < accepter_nums)) {
        void *got_tag;
        bool ok = false;

        GPR_ASSERT(cq.Next(&got_tag, &ok));
        GPR_ASSERT(ok);

        long tag_num = (long)got_tag;

        auto cur_request = holders[tag_num];

        if (cur_request->status->ok()) {
            cout<<" reply accepter id: " << cur_request->reply_packet->server_id();
            if (cur_request->reply_packet->is_error()){
                cout<<" get rejection "<< endl;
                return -1;
            }
            sucess_server_reply_num ++;

            cout<<" reply accepted proposal num " << cur_request->reply_packet->proposal_num(); //min_proposal_num

        } else {
            cout<<"-----------";
            cout<<cur_request->status->error_message();
            cout<<"--------------"<<endl;
            cout << " RPC failed " << cur_request->status->error_message() << endl;
        }
        reply_server_num += 1;
    }

    return 0;

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
        int propose_round = -1;
        bool NACK = true;

        StateMachineCommand returned_cmd;
        while(NACK) { // one census instance
            propose_round ++;
            int propose_num = propose_round * accepter_nums + server_id;
            int acceptedProposal = -1, minProposal;
            int ret = paxos_prepare(entry_id, propose_num, acceptedProposal, returned_cmd); //census protocal phase1;

            if (ret < 0) {// phase 1 fails, increment propose num
                NACK = true;
                continue;
            }

            if (acceptedProposal < 0){//no accpeted values
                returned_cmd.from_client = from_client;
                returned_cmd.client_cmd_id = command_id;
                if (request->is_write()) returned_cmd.cmd = "write";
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
        if (returned_cmd.from_client == from_client && returned_cmd.client_cmd_id == command_id){//end of census
            propose_success = true;//end loop

            //set response
            response->set_is_success(true);
            response->set_server_id(server_id);
            response->set_client_id(from_client);

            response->set_client_command_id(command_id);
            response->set_key(returned_cmd.key);
            response->set_value(returned_cmd.value);
        } else {
            propose_success = false;
            min_empty_log_entry += 1;
        }
    }

    return Status::OK;
}


Status Acceptor::Prepare(::grpc::ServerContext *context, const ::PreparePacket *request, ::PromisePacket *response) {
    lock_guard<mutex> lock(accepter_lock);

    int from_server = request->server_id();
    int entry_id = request->log_entry();
    int proposal_num = request->proposal_num();
    cout<<" Prepare request from server:"<<from_server << " entry: "<<entry_id<<endl;

    if (paxos_instances.find(entry_id) == paxos_instances.end()){
        AccepterEntry log_entry;
        log_entry.acceptedProposal = -1;
        log_entry.minProposal = -1;
        paxos_instances[entry_id] = log_entry;

        response->set_error(false); //promise
        response->set_max_proposal_num(-1); // no previous accepted proposal
    } else {
        auto log_entry = paxos_instances[entry_id];

        if(proposal_num <= log_entry.minProposal){
            response->set_error(true);
        } else{
            log_entry.minProposal = proposal_num;
            response->set_error(false); //promise
            response->set_max_proposal_num(log_entry.acceptedProposal);
            response->set_log_entry(entry_id);

            response->set_client_id(log_entry.acceptedValue.from_client);
            response->set_client_id(log_entry.acceptedValue.client_cmd_id);
            response->set_is_write(log_entry.acceptedValue.cmd == "write");
            response->set_key(log_entry.acceptedValue.key);
            response->set_value(log_entry.acceptedValue.value);
        }

    }

    return Status::OK;
}

Status Acceptor::Propose(::grpc::ServerContext *context, ::ProposePacket *request, ::AcceptPacket *response) {
    lock_guard<mutex> lock(accepter_lock);

    int from_server = request->server_id();
    int entry_id = request->log_entry();
    int proposal_num = request->proposal_num();
    cout<<" Propose request from server:"<<from_server << " entry: "<<entry_id<<endl;

    auto& log_entry = paxos_instances[entry_id];
    if (log_entry.minProposal <= proposal_num){//accept
        response->set_is_error(false);
        response->set_proposal_num(proposal_num);

        log_entry.acceptedProposal = proposal_num;
        if (request->is_write()) log_entry.acceptedValue.cmd = "write";
        else  log_entry.acceptedValue.cmd = "read";

        log_entry.acceptedValue.from_client = request->client_id();
        log_entry.acceptedValue.client_cmd_id = request->client_command_id();

        log_entry.acceptedValue.key = request->key();
        log_entry.acceptedValue.value = request->value();
    } else {
        response->set_is_error(true);
    }

    return Status::OK;
}

