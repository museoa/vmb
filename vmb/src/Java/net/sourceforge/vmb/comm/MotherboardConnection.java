package net.sourceforge.vmb.comm;

import java.io.*;
import java.util.*;
import java.net.*;

public class MotherboardConnection {

	private Socket s;
	private SocketAddress sa;
	private OutputStream outputStream;
	private InputStream in;

    private final long startAddress;
    private final int size;

	private List<IConnectionListener> connectionListeners = new ArrayList<IConnectionListener>();

    public MotherboardConnection(long startAddress, int size) {
        this.startAddress = startAddress;
        this.size = size;
    }

	public void establishConnection(InetAddress address, int port) throws IOException{
	    s = new Socket();
	    sa = new InetSocketAddress(address, port);
	    s.connect(sa);
	    outputStream = s.getOutputStream();
	    in = s.getInputStream();
	    register();
	    new Thread(new Runnable() {
	        public void run() {
	            while(s.isConnected()){
	                try {
	                    dispatchMessage(new Message(MotherboardConnection.this));
	                } catch (IOException e) {
	                    e.printStackTrace();
	                    return;
	                }
	                finally{
	                    try {
	                        s.close();
	                    } catch (IOException e) {
	                        e.printStackTrace();
	                    }
	                }
	            }}}).run();
	}

	public byte readByte() throws IOException{
		int r = in.read();
		if(r < 0){
			throw new IOException("End of stream reached");
		}
		return (byte)(r & 0xFF);
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
				default:
					System.out.println("unhandeled message");
				}
			}
			else{
				switch(message.getId()){
                    case 0:
                        listener.DataReceived((int)(message.getAddress() - startAddress), message.getPayload());
				default:
					System.out.println("unhandeled message");
				}
			}
		}
	}

	private void register() throws IOException {
		send(MessageFactory.createRegistrationMessage(startAddress, startAddress + size, 0xFFl, "framebuffer"));
	}

	public void send(Message message) throws IOException{
		outputStream.write(message.toByteArray());
	}

	public void unregister(){

	}

	public void addConnectionListener(IConnectionListener listener){
		connectionListeners.add(listener);
	}

}
