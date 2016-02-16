package net.sourceforge.vmb.comm;

import java.io.*;
import java.util.*;
import java.net.*;

import org.apache.log4j.*;

public class MotherboardConnection implements IConnection {

	private Socket s;
	private SocketAddress sa;
	private OutputStream outputStream;
	private InputStream in;

    private final long startAddress;
    private final int size;

	private List<IConnectionListener> connectionListeners = new ArrayList<IConnectionListener>();

    protected Logger logger = Logger.getLogger(LoggingConnectionListener.class);
    
    public MotherboardConnection(long startAddress, int size) {
        this.startAddress = startAddress;
        this.size = size;
        PropertyConfigurator.configure("log4j.properties");
    }

	/* (non-Javadoc)
     * @see net.sourceforge.vmb.comm.IConnection#establishConnection(java.net.InetAddress, int)
     */
	public void establishConnection(InetAddress address, int port) throws IOException{
	    s = new Socket();
	    sa = new InetSocketAddress(address, port);
	    s.setTcpNoDelay(true);
	    s.connect(sa);
	    outputStream = s.getOutputStream();
	    in = s.getInputStream();
	    register();
	    new Thread(new Runnable() {
	        public void run() {
	            Logger logger = Logger.getLogger(this.getClass());
	            PropertyConfigurator.configure("log4j.properties");
	            while(s.isConnected()){
	                try {
	                    dispatchMessage(new Message(MotherboardConnection.this));
	                } catch (IOException e) {
	                    logger.warn(e);
	                    return;
	                }
//	                finally{
//	                    try {
//	                        s.close();
//	                    } catch (IOException e) {
//	                        e.printStackTrace();
//	                    }
//	                }
	            }}}).start();
	}

	/* (non-Javadoc)
     * @see net.sourceforge.vmb.comm.IConnection#readByte()
     */
	public byte readByte() throws IOException{
		int r = in.read();
		if(r < 0){
			throw new IOException("End of stream reached");
		}
		byte rr = (byte)(r & 0xFF);
		return rr;
	}

    /*
     * This message will only be called from the receiver thread.
     */
	private void dispatchMessage(Message message) {
		for (IConnectionListener listener : connectionListeners) {
			if(message.isBusMessage()){
				switch(message.getId()){
				case Message.ID_POWERON :
					listener.powerOn();
					break;
				case Message.ID_POWEROFF:
					listener.powerOff();
					break;
				case Message.ID_INTERRUPT:
					listener.interruptRequest(message.getSlot());
					break;
				case Message.ID_RESET:
					listener.reset();
					break;
				case Message.ID_TERMINATE:
				    listener.terminate();
				    break;
				default:
					logger.warn("unhandeled message id " + message.getId());
				}
			}
			else{
				switch(message.getId()){
                    case Message.ID_WRITETETRA:
                        listener.dataReceived((int)(message.getAddress() - startAddress), message.getPayload());
                        break;
				default:
				    logger.warn("unhandeled message id " + message.getId());
				}
			}
		}
	}

	private void register() throws IOException {
		send(MessageFactory.createRegistrationMessage(startAddress, startAddress + size, 0x0l, "framebuffer"));
	}

	public void send(Message message) throws IOException{
		outputStream.write(message.toByteArray());
	}

	public void unregister(){

	}

	/* (non-Javadoc)
     * @see net.sourceforge.vmb.comm.IConnection#addConnectionListener(net.sourceforge.vmb.comm.IConnectionListener)
     */
	public void addConnectionListener(IConnectionListener listener){
		connectionListeners.add(listener);
	}

    /* (non-Javadoc)
     * @see net.sourceforge.vmb.comm.IConnection#shutDown()
     */
    public void shutDown() throws IOException {
        s.close();
    }

}
