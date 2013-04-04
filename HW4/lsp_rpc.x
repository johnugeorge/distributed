/* rpc.x - demonstration of NFS via rpc */


struct packet {
	int conn_id;
	int seq_no;
        string data<1000>;
};


program LSP_PROGRAM {
        version LSP_VERS {
                int sendfn(packet) = 1;
                packet recvfn()=2;

        } = 1;
} = 0x37987898;

