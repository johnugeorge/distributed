import java.io.File;
import java.lang.StringBuffer;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;

public class NetworkFileServerImpl implements NetworkFileServer
{
  public NetworkFileServerImpl()
  {}

  //implementations
  public File lookup(String fileName) throws RemoteException
  { return null; }

  public String getAttr(File file) throws RemoteException
  { return null; }

  public StringBuffer read(File file, int offset, int nBytes, StringBuffer buffer) throws RemoteException
  { return null; }

  public void write(File file, int offset, int nBytes, StringBuffer buffer) throws RemoteException
  { return; }

  public static void main(String[] args)
  {
    try 
    {
      String name = "Network File Server";
      NetworkFileServer fileSystem = new NetworkFileServerImpl();
      NetworkFileServer stub = (NetworkFileServer) UnicastRemoteObject.exportObject(fileSystem, 0);
      Registry nameRegistry = LocateRegistry.getRegistry();
      nameRegistry.rebind(name, stub);
      System.out.println("NetworkFileServer has been bound");
    }
    catch(RemoteException e)
    {
      System.out.println("RMI exception: ");
      e.printStackTrace();
    }
  }
}
