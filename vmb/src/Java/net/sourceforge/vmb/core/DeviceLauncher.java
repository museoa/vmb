package net.sourceforge.vmb.core;

import java.awt.event.*;
import java.io.*;
import java.net.*;

import net.sourceforge.vmb.comm.*;
import net.sourceforge.vmb.view.*;

public class DeviceLauncher {

    private static final int WIDTH  = 640;
    private static final int HEIGHT = 480;


	/**
	 * @param args
	 */
	public static void main(String[] args) {
		final FrameBufferDevice deviceWindow = new FrameBufferDevice(WIDTH, HEIGHT);
		MotherboardConnection connection = new MotherboardConnection(0x0003000000000000l, 4 * WIDTH * HEIGHT);
		connection.addConnectionListener(new IConnectionListener(){
			public void DataReceived(int offset, byte[] payload) {
				System.out.println("data");
			}
			public void interruptRequest(int irqNumber) {
				System.out.println("irq " + irqNumber);
			}
			public void powerOff() {
				System.out.println("poweroff");
			}
			public void powerOn() {
				deviceWindow.setVisible(true);
				deviceWindow.setTitle("Frame Buffer Device");
				deviceWindow.clear();
			}
			public void readRequest() {
				System.out.println("read");
			}
			public void reset() {
				System.out.println("reset");
			}
			public void writeRequest() {
				System.out.println("write");
			}});

		try {
			connection.establishConnection(InetAddress.getByName("mbox.informatik.fh-muenchen.de"), 9002);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

		deviceWindow.setVisible(true);
		deviceWindow.setTitle("Frame Buffer Device");
		deviceWindow.clear();

		deviceWindow.addWindowListener(new WindowListener(){
			public void windowOpened(WindowEvent arg0) {
			}
			public void windowClosing(WindowEvent arg0) {//brutale Methode
				System.exit(0);
				return;
			}
			public void windowClosed(WindowEvent arg0) {
			}
			public void windowIconified(WindowEvent arg0) {
			}
			public void windowDeiconified(WindowEvent arg0) {
			}
			public void windowActivated(WindowEvent arg0) {
			}
			public void windowDeactivated(WindowEvent arg0) {
			}});

        deviceWindow.DataReceived(10*WIDTH + 10, new byte[] {100, 100, 100, 100});
        deviceWindow.DataReceived(11*WIDTH + 11, new byte[] {100, 100, 100, 100});
        deviceWindow.DataReceived(12*WIDTH + 12, new byte[] {100, 100, 100, 100});

	}



}
