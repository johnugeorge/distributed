HW2 - Distributed Password Cracker
=======

### Building the Source ###

To build and run any of the homework projects, navigate to the correct directory and run make:

    $ make

To remove any build artifacts, run:

    $ make clean

### Running the code ###

Three different excutables will be built: server, worker, and request. The server will manage each crack request that come from requestors, and it will distribute the job across all of the connected workers. To run the distributed password cracker, follow the steps below:

1. Start the server:

        $ ./server port

2. Start some workers (recommended one per core that you want to utilitize):

        $ ./worker server_host:port

3. Submit requests to the server:

        $ ./request server_host:port sha1_hash password_length

You can run the workers across many different machines, as well as have multiple workers on a single machine. Each worker is a lightweight process designed to run on a single core. For example, if you have a quad-core processor, and want to devote 50% of your processing power to the password cracking jobs, you should start 2 workers.

The system is designed to be fault-tolerant, so if worker processes become disconnected or killed while they are processing a portion of the crack request, the portion that was lost will be redistributed to another worker. The system was designed so that workers can join or leave at any point in the cracking process.

Servers will remain active until they are killed. Workers will remain active until they are killed or the connection to the server was lost. Requestors will remain active only while a password crack is in progress and the connection to the server can be maintained. The requestor will wait until a result comes back from the server, and it will either say that that password was found or not found. If it was found, the response will be "Found: password", otherwise it will read "Not Found".

The servers and workers log some diagnostic data to standard out while they are running.

An example setup is shown below (all processes on the same host for this example). Assume we want to crack a password "hello", and all passwords are hashed with a SHA1 hash. We can determine the hash for hello by running:
    
    $ echo -n "hello" | openssl sha1
    aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d

So now that we have a SHA1 hash and we know the password length is 5, let's try to crack it. We need to start a few processes (either in the background, or something like screen).

    $ ./server 3000
    $ ./worker localhost:3000
    $ ./worker localhost:3000
    $ ./worker localhost:3000
    $ ./worker localhost:3000
    $ ./request localhost:3000 aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d 5

The request will be submitted to the server which will farm chunks out to each worker. Once the password is found, the request process will print out the result and terminate:

    Found: hello

If you submit an invalid hash (one for which there is no corresponding password of the length you have given), the request process will print "Not Found" and terminate. an example is as follows:

    $ ./request localhost:3000 fakehash 5
    Not Found

