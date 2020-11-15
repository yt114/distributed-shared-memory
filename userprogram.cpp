//
// Created for CSE513 course, Project 1
// EECS Department
// Pennsylvania State University
//

/* This program will create 3 clients and do a read and a write operation with each of them cuncurrently.
 * Please set the varaiable servers before use
 */

#include "client.h"
#include <cstdio>
#include <string>
#include <vector>
#include <ctime>
#include <thread>

typedef unsigned int uint;

#define NUMBER_OF_CLIENTS 	3
#define SIZE_OF_VALUE 		2


int rwratio = 5; //read/write

// Define your server information here
static struct Server_info servers[] = {
		{"35.192.146.152", 50051},
		{"34.74.163.238", 50052},
		{"104.154.25.168", 50053}};

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

	if(argc != 2){
		fprintf(stderr, "%s%s%s\n", "Error\n"
		"Usage: ", argv[0], "[ABD/CM]\n\n"
		"Please note to run the servers first\n"
		"This application is just for your testing purposes, "
		"and the evaluation of your code will be done with another initiation of your Client libraries.");
		return -1;
	}

	if(std::string(argv[1]) == "ABD"){

		// Create ABD clients
		struct Client* abd_clt[NUMBER_OF_CLIENTS];
		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
			abd_clt[i] = client_instance(i, "ABD", servers, sizeof(servers) / sizeof(struct Server_info));
			if(abd_clt[i] == NULL){
				fprintf(stderr, "%s\n", "Error occured in creating clients");
				return -1;
			}
		}
        //performance test
        for (int i=0; i < 20; i++){
            run_test_performance(abd_clt);
        }

        /* //correstness test
        std::vector<std::thread *> threads;

        // Do write operations concurrently
        threads.clear();
        char wvalues[NUMBER_OF_CLIENTS][SIZE_OF_VALUE];
        for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {

            // build a random value
            for (int j = 0; j < SIZE_OF_VALUE; j++) {
                wvalues[i][j] = '0' + rand() % 10;
            }
            // run the thread
            threads.push_back(new std::thread(Thread_helper::_put, abd_clt[i], key, sizeof(key), wvalues[i],
                                              sizeof(wvalues[i])));
        }
        // Wait for all threads to join
        for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {
            threads[i]->join();
        }

        // Do get operations concurrently
        threads.clear();
        char *values[NUMBER_OF_CLIENTS];
        uint32_t value_sizes[NUMBER_OF_CLIENTS];
        for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {
            if (i == 1) {
                for (int j = 0; j < SIZE_OF_VALUE; j++) {
                    wvalues[i][j] = '0' + rand() % 10;
                }
                threads.push_back(new std::thread(Thread_helper::_put, abd_clt[i], key, sizeof(key), wvalues[i],
                                                  sizeof(wvalues[i])));
            } else {
                // run the thread
                threads.push_back(new std::thread(Thread_helper::_get, abd_clt[i], key, sizeof(key), &values[i],
                                                  &value_sizes[i]));
            }
        }
        // Wait for all threads to join
        for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {
            threads[i]->join();
            if (i != 1) delete values[i];
        }
        // remmeber after using values, delete them to avoid memory leak

        threads.clear();
        for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {
            if (i == 0) {
                for (int j = 0; j < SIZE_OF_VALUE; j++) {
                    wvalues[i][j] = '0' + rand() % 10;
                }
                threads.push_back(new std::thread(Thread_helper::_put, abd_clt[i], key, sizeof(key), wvalues[i],
                                                  sizeof(wvalues[i])));
            } else {
                // run the thread
                threads.push_back(new std::thread(Thread_helper::_get, abd_clt[i], key, sizeof(key), &values[i],
                                                  &value_sizes[i]));
            }
        }
        // Wait for all threads to join
        for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {
            threads[i]->join();
            if (i != 0) delete values[i];
        }
        // Clean up allocated memory in struct Client
        for (uint i = 0; i < NUMBER_OF_CLIENTS; i++) {
            if (client_delete(abd_clt[i]) == -1) {
                fprintf(stderr, "%s\n", "Error occured in deleting clients");
                return -1;
            }
        }
        */
	}
	else if(std::string(argv[1]) == "CM"){
		
		// Create CM clients
		struct Client* cm_clt[NUMBER_OF_CLIENTS];
		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
			cm_clt[i] = client_instance(i, "CM", servers, sizeof(servers) / sizeof(struct Server_info));
			if(cm_clt[i] == NULL){
				fprintf(stderr, "%s\n", "Error occured in creating clients");
				return -1;
			}
		}

		// Do write operations concurrently
		std::vector<std::thread*> threads;
        char wvalues[NUMBER_OF_CLIENTS][SIZE_OF_VALUE];
		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
			
			// build a random values
			char value[SIZE_OF_VALUE];
			for(int j = 0; j < SIZE_OF_VALUE; j++){
				wvalues[i][j] = '0' + i;
			}
			threads.push_back(new std::thread(Thread_helper::_put, cm_clt[i], key, sizeof(key), wvalues[i], sizeof(wvalues[i])));
	    }
	    // Wait for all threads to join
	    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
	    	threads[i]->join();
	    }
		
		// Do get operations concurrently
		threads.clear();
		char* values[NUMBER_OF_CLIENTS];
		uint32_t value_sizes[NUMBER_OF_CLIENTS];
		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
			
			// run the thread
			threads.push_back(new std::thread(Thread_helper::_get, cm_clt[i], key, sizeof(key), &values[i], &value_sizes[i]));
	    }
	    // Wait for all threads to join
	    for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
	    	threads[i]->join();
            delete values[i];
	    }
	    // remmeber after using values, delete them to avoid memory leak

		// Clean up allocated memory in struct Client
		for(uint i = 0; i < NUMBER_OF_CLIENTS; i++){
			if(client_delete(cm_clt[i]) == -1){
				fprintf(stderr, "%s\n", "Error occured in deleting clients");
				return -1;
			}
		}

	}
	else{
		fprintf(stderr, "%s%s%s\n", "Error\n"
		"Usage: ", argv[0], "[ABD/CM]\n\n"
		"Please note to run the servers first\n"
		"This application is just for your testing purposes, "
		"and the evaluation of your code will be done with another initiation of your Client libraries.");
		return -1;
	}

	return 0;
}