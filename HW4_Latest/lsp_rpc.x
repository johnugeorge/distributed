/* rpc.x - demonstration of NFS via rpc */


struct LSPMessage1 {
	int connid;
	int seqnum;
        string payload<1000>;
};


program LSP_PROGRAM {
        version LSP_VERS {
                int sendfn(LSPMessage1) = 1;
                LSPMessage1 recvfn(int)=2;
		void callbackfn(int)  = 3;

        } = 1;
} = 0x37987898;

