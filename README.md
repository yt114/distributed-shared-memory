# distributed-shared-memory

Complete network 

## Procedure communicate

A client utilizes the communicate primitive to broadcast a message <M> to all the server processors  and then to collect <ACK> messages from a majority of servers. Communication between client i and server j is send-and-wait. Communicate procedure is run on top of TCP and thus FIFO order and reliable delivery is guranteed. 

design 1-synchronized IO:
To implement the broadcast<M>, swap #servers threads and each thread synchronizedly sends <M> and recvs <ACK> from one server.
  
design 2-asynchronized IO: which is the procedure in [1]. source from google rpc asynchronized IO
In this project, asynchronized send method is used. In each communicate procedure, the deadline of each rpc call is set to time_out = 5000ms. A comminicate procedire either returns key-value pair when receives from a majority server or return fail results when exceeds bounded deadline. This deadline serves as failure detection and bounds the waiting time for a single communicate procedure.

## Procedure record
Mutiple-writer atomic register needs extract step to pick timestamp in contrast of single-writer register which can track latest timestamp in client processor's memory. In [1], client utilizes procedure label to fetch largest timestamp from the atomic register system consists of server quorums. Note, the timestamp is unbounded integer. 

## Procedure Ack

design 1- synchronized IO: each server process can only serve one Read/write at one time.
A global mutex lock is used to protect critical section. 

design 2- asynchronized IO:
asynchronized IO actually allows both client and server run in single thread with moderate performance. 

# Server side states
Each maintains a hashmap and each key in the hashmap is associated with state tuple(Logic time t, client id, value), where client id is used to bread tie of logic time t. 

Client call update procedure (key, logic time t, client id, value):
server update key value if timestamp consists of (t, id) is larger than current timestamp.

Client call query procedure (key):
server returns the associated tuple (logic time t, client id, value).


Reference:
[1] Sharing memory robustly in message-passing systems. H Attiya, A Bar-Noy, D Dolev. Journal of the ACM 42 (1), 124-142, 1995. 638, 1995.
eate an instance of the factory ServerBuilder class.
Specify the address and port we want to use to listen for c
