package org.apache.hadoop.hdfs.server.fastcopy;

import java.io.IOException;

/**
 * Generic exception type for the fastcopy tool.
 */
public class FastCopyException extends IOException
{
  public static final long serialVersionUID = 1L;
  
  public FastCopyException(String msg) 
  {
    super(msg);
  }
}

