Implementation of Fast Copy for HDFS

Advantages
1.FastCopy takes only 10% of time taken for existing copy tool 'cp' command.
2.The number of messages needed for FastCopy does not depend on file size 
and is proportional to number of replicas needed(3-4). But,
 number of network messages needed for traditional copy depends on file size.

The fast copy mechanism for a file works as
follows :
1) Query metadata for all blocks of the source file.
2) For each block 'b' of the file, find out its datanode locations.
3) For each block of the file, add an empty block to the namesystem for
the destination file.
4) For each location of the block, instruct the datanode to make a local
copy of that block.
5) Once each datanode has copied over its respective blocks, they
report to the namenode about it.
6) Wait for all blocks to be copied and exit.
This would speed up the copying process considerably by removing top of
the rack data transfers.
