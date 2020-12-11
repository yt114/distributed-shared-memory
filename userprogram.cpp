//
// Created for CSE513 course, Project 1
// EECS Department
// Pennsylvania State University
//

/* This program will create 3 clients and do a read and a write operation with each of them cuncurrently.
 * Please set the varaiable servers before use
 */

#include "paxos_client.h"
#include <cstdio>
#include <string>
#include <vector>
#include <ctime>
#include <thread>

typedef unsigned int uint;

#define NUMBER_OF_CLIENTS 	3
#define SIZE_OF_VALUE 		2
#define TEST_IT 100

int rwratio = 1; // read:write = rwratio : 1-0.1*rwratio

// Define your server information here
static struct Server_info servers[] = {
		{"127.0.0.1", 50051}};

static char key[] = "key1"; // We only have one key in this userprogram

namespace Thread_helper{
	void _put(const struct Client* c, const char* key, uint32_t key_size, const char* value, uint32_t value_size){
		
		int status = put(c, key, key_size, value, value_size);

		if(status == 0){ // Success
			return;
		}
		else{
			exit(-1);
		}

		return;
	}

	void _get(const struct Client* c, const char* key, uint32_t key_size, char** value, uint32_t *value_size){

		int status = get(c, key, key_size, value, value_size);

		if(status == 0){ // Success
			return;
		}
		else{
			exit(-1);
		}

		return;
	}
}

void run_test_performance(struct Client* abd_clt[]){
    std::vector<std::thread *> threads;

    // Do write operations concurrently
    char wvalues[NUMBER_OF_CLIENTS][SIZE_OF_VALUE];
    bool isread[NUMBER_OF_CLIENTS];
    char *values[NUMBER_OF_CLIENTS];
    uint32_t value_sizes[NUMBER_OF_CLIENTS];

    for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {

        // build a random value
        int rv = rand() % 10;
        if (rv >= rwratio) { //write
            for (int j = 0; j < SIZE_OF_VALUE; j++) {
                wvalues[i][j] = '0' + rand() % 10;
            }
            // run the thread
            threads.push_back(new std::thread(Thread_helper::_put, abd_clt[i], key, sizeof(key), wvalues[i],
                                              sizeof(wvalues[i])));
            isread[i] = false;
        } else{
            threads.push_back(new std::thread(Thread_helper::_get, abd_clt[i], key, sizeof(key), &values[i],
                                              &value_sizes[i]));
            isread[i] = true;
        }
    }
    // Wait for all threads to join
    for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {
        threads[i]->join();
        if (isread[i]) delete values[i];
    }
}

int main(int argc, char* argv[]){
    // Create CM clients
    struct Client* clt[NUMBER_OF_CLIENTS];
    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
        clt[i] = client_instance(i, "P", servers, sizeof(servers) / sizeof(struct Server_info));
        if(clt[i] == NULL){
            fprintf(stderr, "%s\n", "Error occured in creating clients");
            return -1;
        }
    }

    char wval[3] = "11";
    put(clt[0], key, sizeof(key), wval, sizeof(wval));

    //performance test
//    for (int i=0; i < TEST_IT; i++){
//        run_test_performance(cm_clt);
//    }
//		// Do write operations concurrently
//		std::vector<std::thread*> threads;
//        char wvalues[NUMBER_OF_CLIENTS][SIZE_OF_VALUE];
//		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
//
//			// build a random values
//			char value[SIZE_OF_VALUE];
//			for(int j = 0; j < SIZE_OF_VALUE; j++){
//				wvalues[i][j] = '0' + i;
//			}
//			threads.push_back(new std::thread(Thread_helper::_put, cm_clt[i], key, sizeof(key), wvalues[i], sizeof(wvalues[i])));
//	    }
//	    // Wait for all threads to join
//	    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
//	    	threads[i]->join();
//	    }
//
//		// Do get operations concurrently
//		threads.clear();
//		char* values[NUMBER_OF_CLIENTS];
//		uint32_t value_sizes[NUMBER_OF_CLIENTS];
//		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
//
//			// run the thread
//			threads.push_back(new std::thread(Thread_helper::_get, cm_clt[i], key, sizeof(key), &values[i], &value_sizes[i]));
//	    }
//	    // Wait for all threads to join
//	    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
//	    	threads[i]->join();
//            delete values[i];
//	    }
//	    // remmeber after using values, delete them to avoid memory leak
//
//		// Clean up allocated memory in struct Client
//		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
//			if(client_delete(cm_clt[i]) == -1){
//				fprintf(stderr, "%s\n", "Error occured in deleting clients");
//				return -1;
//			}
//		}


	return 0;
}