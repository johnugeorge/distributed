package org.apache.hadoop.hdfs.server.fastcopy;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Formatter;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hdfs.server.common.HdfsConstants;
import org.apache.hadoop.hdfs.server.common.Util;
import org.apache.hadoop.hdfs.DFSClient;
import org.apache.hadoop.hdfs.DistributedFileSystem;
import org.apache.hadoop.hdfs.protocol.*;
import org.apache.hadoop.hdfs.protocol.FSConstants.DatanodeReportType;
import org.apache.hadoop.hdfs.security.token.block.BlockTokenIdentifier;
import org.apache.hadoop.hdfs.security.token.block.BlockTokenSecretManager;
import org.apache.hadoop.hdfs.security.token.block.ExportedBlockKeys;
import org.apache.hadoop.hdfs.server.datanode.DataNode;
import org.apache.hadoop.hdfs.server.namenode.NameNode;
import org.apache.hadoop.hdfs.server.protocol.NamenodeProtocol;
import org.apache.hadoop.hdfs.server.protocol.BlocksWithLocations.BlockWithLocations;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.io.IOUtils;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.retry.RetryPolicies;
import org.apache.hadoop.io.retry.RetryPolicy;
import org.apache.hadoop.io.retry.RetryProxy;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.RemoteException;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.net.NetworkTopology;
import org.apache.hadoop.security.UserGroupInformation;
import org.apache.hadoop.security.token.Token;
import org.apache.hadoop.util.Daemon;
import org.apache.hadoop.util.StringUtils;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;



public class FastCopy implements Tool {
	private static final Log LOG = 
		LogFactory.getLog(FastCopy.class.getName());
	final private static long MAX_BLOCKS_SIZE_TO_FETCH = 2*1024*1024*1024L; //2GB

	/** The maximum number of concurrent blocks moves for 
	 *    * balancing purpose at a datanode
	 *       */
	public static final int MAX_NUM_CONCURRENT_MOVES = 5;

	private Configuration conf;
        private DFSClient dfsClient;
	private NamenodeProtocol namenode;
	private ClientProtocol client;
	private ClientProtocol nameNode;
	private FileSystem fs;
	private boolean isBlockTokenEnabled;
	private boolean shouldRun;
	private long keyUpdaterInterval;
	private BlockTokenSecretManager blockTokenSecretManager;
	private Daemon keyupdaterthread = null; // AccessKeyUpdater thread
	private final static Random r = new Random();

	/* Given elaspedTime in ms, return a printable string */
	private static String time2Str(long elapsedTime) {
		String unit;
		double time = elapsedTime;
		if (elapsedTime < 1000) {
			unit = "milliseconds";
		} else if (elapsedTime < 60*1000) {
			unit = "seconds";
			time = time/1000;
		} else if (elapsedTime < 3600*1000) {
			unit = "minutes";
			time = time/(60*1000);
		} else {
			unit = "hours";
			time = time/(3600*1000);
		}

		return time+" "+unit;
	}

	/** return this FastCopy's configuration */
	public Configuration getConf() {
		return conf;
	}

	/** set this FastCopy's configuration */
	public void setConf(Configuration conf) {
		this.conf = conf;
	}

	final public static int SUCCESS = 1;
	final public static int FAILURE = 2;

	public int run(String[] args) throws Exception {
		long startTime = Util.now();
		OutputStream out = null;
		try {
			parseArgs(args);
			String src=args[0];
			String dest=args[1];
			init();
			copyBlocks(src,dest);
			return SUCCESS;
		}catch (IllegalArgumentException ae) {
			System.out.println("Received an Illegal Argument exception: " + ae.getMessage());
			System.exit(-1);

		} catch (IOException e) {
			System.out.println("Received an IO exception: " + e.getMessage() +
					" . Exiting...");
			System.exit(-1);
		} 
		return FAILURE;
	}

	private void copyBlocks(String src,String dest) throws IOException {
		System.out.println(" Source fileName "+src);
		System.out.println(" Destination fileName "+dest);
		Path srcPath = new Path(src);
		FileSystem srcFs = srcPath.getFileSystem(getConf());
		FileStatus srcGlob[] = srcFs.globStatus(srcPath);
		if (null == srcGlob)
		{
			System.out.println(" Source file doesn't exist.");
			System.exit(-1);
		}
		Path destPath = new Path(dest);
		//FileSystem destFs = destPath.getFileSystem(getConf());	
		FileSystem destFs= new DistributedFileSystem(NameNode.getServiceAddress(conf, true) ,getConf());
//		 ((DistributedFileSystem)destFs).setVerifyChecksum(false);
		this.dfsClient= ((DistributedFileSystem)destFs).getDFSClient();
		this.nameNode= dfsClient.getNameNode();
		System.out.println(" dfsClient Name "+dfsClient.getClientName());
		FileStatus destGlob[] = destFs.globStatus(destPath);
		if (destGlob != null)
		{
			System.out.println(" Destination file already exist");
			System.exit(-1);
		}
		FSDataOutputStream fs= destFs.create(destPath);
		LocatedBlocks srcLocations = client.getBlockLocations(src,0,
				Long.MAX_VALUE);
		for(LocatedBlock sourceBlock: srcLocations.getLocatedBlocks())
		{
			for(DatanodeInfo dnInfo : sourceBlock.getLocations())
			{
				System.out.println(" Priting Src Block locations:: "+dnInfo.getDatanodeReport());

			}
			String taskId = conf.get("mapred.task.id");
			System.out.println(" Client Name "+dfsClient.getClientName());
			System.out.println(" source Block size "+sourceBlock.getBlock().getNumBytes());
			System.out.println(" source Block Id "+sourceBlock.getBlock().getBlockId());
			System.out.println(" source Block "+sourceBlock.getBlock());
			long fileSize=sourceBlock.getBlock().getNumBytes();
			ArrayList<DatanodeInfo> excludedNodes = new ArrayList<DatanodeInfo>();

			DatanodeInfo[] excluded = excludedNodes.toArray(new DatanodeInfo[0]);

			LocatedBlock destBlock= nameNode.addBlockLocations(dest, dfsClient.getClientName() , excluded,sourceBlock.getLocations()); 	  
			//LocatedBlock destBlock= nameNode.addBlock(dest, dfsClient.getClientName() , excluded); 	  
 			DatanodeInfo[] dnInfo = destBlock.getLocations();
			//for(DatanodeInfo dnInfo : destBlock.getLocations())
			{
				System.out.println(" Priting Dest Block locations:: "+dnInfo[0].getDatanodeReport());
                                connect_DataNode(dnInfo,sourceBlock,destBlock,dfsClient.getClientName(),fileSize);			

			}
		}
		fs.close();


	}


	private void connect_DataNode(DatanodeInfo[] destDnInfo,LocatedBlock srcBlock,LocatedBlock destBlock,String clientName,long fileSize)
	{
		Socket sock = new Socket();
		DataOutputStream out = null;
		DataInputStream in = null;
		try {
			sock.connect(NetUtils.createSocketAddr(
						destDnInfo[0].getName()), HdfsConstants.READ_TIMEOUT * destDnInfo.length);
			sock.setKeepAlive(true);
			out = new DataOutputStream( new BufferedOutputStream(
						sock.getOutputStream(), FSConstants.BUFFER_SIZE));
			sendRequest(out,destDnInfo,srcBlock,destBlock,clientName,fileSize);
			in = new DataInputStream( new BufferedInputStream(
						sock.getInputStream(), FSConstants.BUFFER_SIZE));
			receiveResponse(in);
		} catch (IOException e) {
		        System.out.println(" IOException for Dest"+destDnInfo);
		}
	       	finally {
			IOUtils.closeStream(out);
			IOUtils.closeStream(in);
			IOUtils.closeSocket(sock);

		}
	}


	private void receiveResponse(DataInputStream in) throws IOException {
		short status = in.readShort();
		if (status != DataTransferProtocol.OP_STATUS_SUCCESS) {
			if (status == DataTransferProtocol.OP_STATUS_ERROR_ACCESS_TOKEN)
				throw new IOException("block move failed due to access token error");
			throw new IOException("block move is failed");
		}
		else
		{
			System.out.println(" Response Success ");
		}
	}

	/* Send a block replace request to the output stream*/
	private void sendRequest(DataOutputStream out,DatanodeInfo[] nodes,LocatedBlock srcBlock,LocatedBlock destBlock,String clientName,long fileSize) throws IOException {
		out.writeShort(DataTransferProtocol.DATA_TRANSFER_VERSION);
		out.writeByte(DataTransferProtocol.OP_LOCAL_COPY);
		out.writeLong(srcBlock.getBlock().getBlockId());
		out.writeLong(srcBlock.getBlock().getGenerationStamp());
		out.writeLong(destBlock.getBlock().getBlockId());
		out.writeLong(destBlock.getBlock().getGenerationStamp());
		Text.writeString(out, clientName);
		out.writeLong(fileSize);
		Token<BlockTokenIdentifier> sourceAccessToken = BlockTokenSecretManager.DUMMY_TOKEN;
		if (isBlockTokenEnabled) {
			sourceAccessToken = blockTokenSecretManager.generateToken(null, srcBlock.getBlock(), 
					EnumSet.of(BlockTokenSecretManager.AccessMode.REPLACE,
						BlockTokenSecretManager.AccessMode.COPY));
		}
		Token<BlockTokenIdentifier> destAccessToken = BlockTokenSecretManager.DUMMY_TOKEN;
		if (isBlockTokenEnabled) {
			destAccessToken = blockTokenSecretManager.generateToken(null, destBlock.getBlock(), 
					EnumSet.of(BlockTokenSecretManager.AccessMode.REPLACE,
						BlockTokenSecretManager.AccessMode.COPY));
		}
		out.writeInt( nodes.length -1);
		for (int i = 1; i < nodes.length; i++) {
			nodes[i].write(out);
		}
		//sourceAccessToken.write(out);
		//destAccessToken.write(out);
		out.flush();
	}

	/* Build a NamenodeProtocol connection to the namenode and
	 *    * set up the retry policy */ 
	private static NamenodeProtocol createNamenode(Configuration conf)
		throws IOException {
		InetSocketAddress nameNodeAddr = NameNode.getServiceAddress(conf, true);
		RetryPolicy timeoutPolicy = RetryPolicies.exponentialBackoffRetry(
				5, 200, TimeUnit.MILLISECONDS);
		Map<Class<? extends Exception>,RetryPolicy> exceptionToPolicyMap =
			new HashMap<Class<? extends Exception>, RetryPolicy>();
		RetryPolicy methodPolicy = RetryPolicies.retryByException(
				timeoutPolicy, exceptionToPolicyMap);
		Map<String,RetryPolicy> methodNameToPolicyMap =
			new HashMap<String, RetryPolicy>();
		methodNameToPolicyMap.put("getBlocks", methodPolicy);
		methodNameToPolicyMap.put("getAccessKeys", methodPolicy);

		UserGroupInformation ugi = UserGroupInformation.getCurrentUser();

		return (NamenodeProtocol) RetryProxy.create(
				NamenodeProtocol.class,
				RPC.getProxy(NamenodeProtocol.class,
					NamenodeProtocol.versionID,
					nameNodeAddr,
					ugi,
					conf,
					NetUtils.getDefaultSocketFactory(conf)),
				methodNameToPolicyMap);
	}

	private void init() throws IOException {

		this.namenode = createNamenode(conf);
		this.client = DFSClient.createNamenode(conf);
		this.fs = FileSystem.get(conf);
		ExportedBlockKeys keys = namenode.getBlockKeys();
		this.isBlockTokenEnabled = keys.isBlockTokenEnabled();
		if (isBlockTokenEnabled) {
			long blockKeyUpdateInterval = keys.getKeyUpdateInterval();
			long blockTokenLifetime = keys.getTokenLifetime();
			LOG.info("Block token params received from NN: keyUpdateInterval="
					+ blockKeyUpdateInterval / (60 * 1000) + " min(s), tokenLifetime="
					+ blockTokenLifetime / (60 * 1000) + " min(s)");
			this.blockTokenSecretManager = new BlockTokenSecretManager(false,
					blockKeyUpdateInterval, blockTokenLifetime);
			this.blockTokenSecretManager.setKeys(keys);

		}
	}


	private void parseArgs(String[] args) {
		int argsLen = (args == null) ? 0 : args.length;
		System.out.println(" Argument length "+ argsLen);
		if(argsLen !=2)
		{
			System.out.println(" Argument length "+ argsLen +" . Expected usage: bin/hadoop fastcopy <src> <dest>");
			System.exit(-1);
		}
	}


	public static void main(String[] args) {
		try {
			System.exit( ToolRunner.run(null, new FastCopy(), args) );
		} catch (Throwable e) {
			LOG.error(StringUtils.stringifyException(e));
			System.exit(-1);
		}

	}


	/** Default constructor */
	FastCopy() {
	}

	/** Construct a FastCopy from the given configuration */
	FastCopy(Configuration conf) {
		setConf(conf);
	} 

}

