package edu.fhm.mmixmb.core;

import java.awt.event.*;
import java.io.*;
import java.net.*;

import edu.fhm.mmixmb.comm.*;
import edu.fhm.mmixmb.view.*;

public class DeviceLauncher {
	

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		final FrameBufferDevice deviceWindow = new FrameBufferDevice();
		MotherboardConnection connection = new MotherboardConnection();
		connection.addConnectionListener(new IConnectionListener(){
			public void DataReceived() {
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
				deviceWindow.setTitle("MMIX Frame Buffer");
				deviceWindow.display(genImage());
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
		deviceWindow.setTitle("MMIX Frame Buffer");
		deviceWindow.display(genImage());

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
	}

	private static int[][] genImage() {
		int[][] raster = new int[640][480];
		for (int i = 0; i < raster.length; i++) {
			for (int j = 0; j < raster[i].length; j++) {
				raster[i][j] = 0x123456;
			}
		}
		return raster;
	}

}
