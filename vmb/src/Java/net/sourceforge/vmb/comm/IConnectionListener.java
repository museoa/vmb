package net.sourceforge.vmb.comm;

public interface IConnectionListener {

	public void dataReceived(int offset, byte[] payload);
	public void powerOn();
	public void powerOff();
	public void reset();
	public void readRequest();
	public void writeRequest();
	public void interruptRequest(int irqNumber);

}
