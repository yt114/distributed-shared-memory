# distributed-shared-memory

## Project Summary
**Config**

Server address on client side is defined at the beginning of `userprogram.c` in `static struct Server_info servers[]`.

Server address on server side is defined at `server.h` in ` string server_addrs[]`.

**Run server**

The compiled `server` takes 3 arguments:
1. the server id starting from 0.
2. the server address in `<ip>:<port>`.
3. the Shared memory protocal. "ABD" for linearizable shared memory and "CM" for casual memory. 

Ex: `$./server 0 0.0.0.0:50051 CM`


**Run client**

The `client.h` defines `put(...), get(...)` interface for a client to write/read a key/value from the server quorum.
The `userprogram.c` contains `main` function for testing. Compile using `makefile` or cmake gives a `client` executable file. 
run `client` by `$./client ABD` for linearizable SM or `$./client CM` casual SM. 

## Linearizable Shared Memory
Complete network.

We treat each key-value pair as a object/automatic register. 

### Procedure communicate

A client utilizes the communicate primitive to broadcast a message <M> to all the server processors  and then to collect <ACK> messages from a majority of servers. Communication between client i and server j is send-and-wait. Communicate procedure is run on top of TCP and thus FIFO order and reliable delivery is guranteed. 

design 1-synchronized IO:
To implement the broadcast<M>, swap #servers threads and each thread synchronizedly sends <M> and recvs <ACK> from one server.
  
design 2-asynchronized IO: which is the procedure in [1]. source from google rpc asynchronized IO
In this project, asynchronized send method is used. In each communicate procedure, the deadline of each rpc call is set to time_out = 5000ms. A comminicate procedire either returns key-value pair when receives from a majority server or return fail results when exceeds bounded deadline. This deadline serves as failure detection and bounds the waiting time for a single communicate procedure.

### Procedure record
Mutiple-writer atomic register needs extract step to pick timestamp in contrast of single-writer register which can track latest timestamp in client processor's memory. In [1], client utilizes procedure label to fetch largest timestamp from the atomic register system consists of server quorums. Note, the timestamp is unbounded integer. 

### Procedure Ack

design 1- synchronized IO: each server process can only serve one Read/write at one time.
A global mutex lock is used to protect critical section. 

design 2- asynchronized IO:
asynchronized IO actually allows both client and server run in single thread with moderate performance. 

In this project, grpc async server is implemented in a single thread to handle linearizable read/write request.

### Server side protocols
Server side states: Each maintains a hashmap and each key in the hashmap is associated with state tuple(Logic time t, client id, value), where client id is used to bread tie of logic time t. 

Client call update procedure (key, logic time t, client id, value):
server update key value if timestamp consists of (t, id) is larger than current timestamp.


Client call query procedure (key):
server returns the associated tuple (logic time t, client id, value).

correstness gurantees: linearlizability is composable. 
##testing 
### Correstness
Here, number of servers is set as 3. The client process forks three concurrent threads performs mixed put (i.e. write(key)) and get (read a keys value) and such operation are repeated for mutiple rounds. Each thread is uniquely identified by client id. This step generates some log{client id}.txt files which are threads' local history which consists of time of operations' invoke and finish. They local histories are merged into a global history manually according to the time. Note these client threads are run on a single machine such that no clock drifting issue is concerned. 


[knossos!](https://github.com/jepsen-io/knossos) available on github is used to check whether a history file log.edn is linearizable or not by running:

```bash
  lein run --model cas-register <log.edn>
```
There are some logs generated in `testcase/linear_logs/` which pass checking. 

## performance
| #server | write ratio           | 0.1    | 0.5    | 0.9    |
|---------|-----------------------|--------|--------|--------|
| 3       | average latency (sec) | 0.3445 | 0.3464 | 0.3465 |
| 5       | average latency (sec) | 8.1   | 8.12  | 8.11  |

This performace is to test client average request time connecting to these five servers spread in different area: 1.us−east, 2.us−central, 3.europe−west, 4. asia −east 5. europe−north. 

When server number is 3, 1.us−east, 2.us−central, 3.europe−west are used. 
When server number is 5, servers outside us responses latency are more than 5s which incurs many timeout request. As a result, few request success for 5 server cases. 
  


## Casual order shared memory
Implements shared memory algorithm proposed in [2], which gurantees of concurrent operations' causal order and non-blocking property.

CM := for each process i, there is a serialization $S_{i}$ of process i local history and all writes operation in system that respects casual order. 
Reference:


## performance
| #server | write ratio           | 0.1    | 0.5    | 0.9    |
|---------|-----------------------|--------|--------|--------|
| 3       | average latency (sec) | 0.1699 | 0.1929 | 0.1986 |
| 5       | average latency (sec) | 0.21065 | 0.2573 | 0.28485 |

### structure
OutQueue: A FIFO queue contains write tuple which is <process id, key, value, vector time>
Inqueue: A priority queue in ascending ordered  of write tuple's vector time 


## Paxos
In this project, I implemented a Paxos and runs a Linearizability distributed shared memory on top of that. 

**Run server**
the compiled server takes four arguments
1. the proposer id, an int value
2. the proposer address in <IP>:<port>
3. the accepter id, an int value
4. the accepter address in <IP>:<port>
  
 
Ex: `$./PaxosServer 0 0.0.0.0:50051 0 0.0.0.0:50052`

Once initilization is done, the process outputs:

`Accepter Server id:0 listening on 0.0.0.0:50052
Proposer Server id:0 listening on 0.0.0.0:50051
`
### Basic Paxos
#### Client side
**stage 1**: A clients sends a get/put request to its local server(acts as a proposer). The command is the value for the system to reach consensus on. 

**stage 2**: the clients blocks until server returns.

**stage 3** the call finishes.

#### Server side
a proposer thread, which takes client's request and communicate with accepters. A proposer thread consist of 

1. **state machine log**
2. **memory cache**

When a client request comes, it picks a minimal empty log entry and run poaxs instance. If a command is agreed on that entry, the proposr learns that by proposing a learned value. Then it executes that command. Then proposer increments empty log entry by 1, and run again until the client request is commited.

**stage 1**: upon get a new request from a client, the proposal find an empty log entry of smallest index. Each command is assigned a unique ID as (clientID, localcommandID), where local commandID is a monotonactily increaseing int local variable. The proposer then sends a preposal (command, proposal num) to accepters. 

**stage 2** After consensus phase completes, if return command ID is same as the requested command ID, a log entry is achieved consensus. Else the next log entry is found and the concensus protocal is repeated. Eiter case, the returned command are executed in StateMachine.  

##### Concensus protocol
**Phase 1**: broadcasr Prepare RPCs. Proposer sends (command, proposal num) to all acceptor.

**Phase 2** Acceptor responses to Prepare RPC. 
```
Init: minProposal = -1, acceptedProposal = -1, acceptedValue = NULL.

Prepare RPC(proposal Num):
  if proposal Num > minProposal:
    minProposal = proposal Num
    return (acceptedProposal, acceptedValue)
  else:
    return NACK
```
**Phase 3**: if NACK Proposer increment proposal number and go to phase1. Else broadcast Accept(proposal num, value).

**Phase 4**: Accepter returns minProposal = n if old_minProposal <= proposer's proposal num. Else it returns rejection.

[1] Sharing memory robustly in message-passing systems. H Attiya, A Bar-Noy, D Dolev. Journal of the ACM 42 (1), 124-142, 1995. 638, 1995.

[2] Causal memory: definitions, implementation, and programming. 
