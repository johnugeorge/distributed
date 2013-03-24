import java.io.File;
import java.lang.StringBuffer;
import java.rmi.Remote;
import java.rmi.RemoteException;

public interface NetworkFileServer extends Remote
{
  public File lookup(String fileName) throws RemoteException;

  public String getAttr(File file) throws RemoteException;

  public StringBuffer read(File file, int offset, int nBytes, StringBuffer buffer) throws RemoteException;

  public void write(File file, int offset, int nBytes, StringBuffer buffer) throws RemoteException;
}
