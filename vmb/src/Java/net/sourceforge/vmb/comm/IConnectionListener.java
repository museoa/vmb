package edu.fhm.mmixmb.comm;

public interface IConnectionListener {
	
	public void DataReceived();
	public void powerOn();
	public void powerOff();
	public void reset();
	public void readRequest();
	public void writeRequest();
	public void interruptRequest(int irqNumber);

}
