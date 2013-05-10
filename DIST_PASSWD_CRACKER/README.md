The Project description is given in Project_Desc.pdf

The design of the distributed password cracker was to split each job into small,
fixed-size chunks that can be handled by single-threaded, lightweight worker
processes. Each worker only works on one chunk at a time, but it should be
very efficient at computing all of the hashes for that chunk. One benefit of
the lightweight worker processes are that it gives the user much more flexibility
in how they want to run the distributed system. For example, if you have a
quad-core processor on a computer that you use for everyday work and internet
browsing, you probably don’t fully utilize your processing power. Each of our
worker threads is designed to fully utilize a single core, which allows the user
to choose how much of their processing power they will devote to the password
cracking jobs. If they want to use 50% of their processing power, they could
spawn two worker processes.
The server handles the bulk of the work distributing and managing the
pending jobs. Jobs are partitioned into fixed-size chunks, and chunks are handed
out to available workers one at a time. If multiple crack requests are currently
pending, one chunk is taken from each job in a round-robin order so that all
jobs can be processed concurrently. Lost chunks (ones where the worker who is
working on a chunk losses connection or gets killed) are placed at the end of the
queue and picked up by another worker later on.



The LSP Protocol implementation follows the guidelines laid out in the orig-inal handout. 
It allows for reliable ordering of packets and retransmission to
guarantee delivery. It does not compute checksums of the packets, so there is
no guarantee on the validity of packets that arrive. The LSP Protocol is split
into two components: the server and the client. Each client maintains a single
connection to a server, while a server can have zero or more connections from
clients. Therefore, the server must keep track of all of the individual clients and
communicate with each one separately.
The general design is to send and receive messages as fast as possible using
user-level read and write threads in conjunction with an epoch thread. Therefore,
each client and server will maintain three additional threads. The epoch
thread is built as outlined in the protocol description. The read and write
threads utilize inbox and outbox queues and use mutexes for thread synchro-nization. 
The underlying network IO is performed asynchronously so that we
can make sure the connection stays valid while data is trying to be read. The
LSP API, however, is designed so that all calls are blocking, since this makes it
easier for the user to implement common programs
