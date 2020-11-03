//
// Created by ye on 10/26/20.
//

#include "server.h"

int main(int argc, char** argv){
    ServerID = atoi(argv[1]);
    string server_addr(argv[2]);
    RunServer(server_addr);
    return 0;
}