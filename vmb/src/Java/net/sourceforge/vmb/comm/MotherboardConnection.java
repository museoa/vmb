package edu.fhm.mmixmb.comm;

import java.io.*;
import java.util.*;
import java.net.*;

public class MotherboardConnection {

	private Socket s;
	private SocketAddress sa;
	private OutputStream outputStream;
	private InputStream in;

	private List<IConnectionListener> connectionListeners = new ArrayList<IConnectionListener>();

	public void establishConnection(InetAddress address, int port) throws IOException{
		s = new Socket();
		sa = new InetSocketAddress(address, port);
		s.connect(sa);
		outputStream = s.getOutputStream();
		in = s.getInputStream();
		try{
			register();
			while(s.isConnected()){
				dispatchMessage(new Message(this));
			}
		}catch(IOException ex){
			// TODO handle exception
			ex.printStackTrace();
		}
		finally{
			s.close();
		}
	}
	
	public byte readByte() throws IOException{
		int r = in.read();
		if(r < 0){
			throw new IOException("End of stream reached");
		}
		return (byte)(r & 0xFF);
	}

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
				default:
					System.out.println("unhandeled message");
				}					
			}
		}
	}

	private void register() throws IOException {
		send(MessageFactory.createRegistrationMessage(0x0003000000000000l, 0x0003000000100000l, 0xFFl, "framebuffer"));
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
