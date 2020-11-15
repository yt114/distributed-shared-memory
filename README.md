# distributed-shared-memory

## Project structure
**Config**

Server address on client side is defined at the beginning of `userprogram.c` in `static struct Server_info servers[]`.

Server address on server side is defined at `server.h` in ` string server_addrs[]`.

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

### structure
OutQueue: A FIFO queue contains write tuple which is <process id, key, value, vector time>
Inqueue: A priority queue in ascending ordered  of write tuple's vector time 

[1] Sharing memory robustly in message-passing systems. H Attiya, A Bar-Noy, D Dolev. Journal of the ACM 42 (1), 124-142, 1995. 638, 1995.
[2] Causal memory: definitions, implementation, and programming. 
