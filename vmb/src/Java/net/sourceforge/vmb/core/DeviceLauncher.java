package net.sourceforge.vmb.core;

import java.awt.event.*;
import java.io.*;
import java.net.*;

import javax.swing.*;

import net.sourceforge.vmb.comm.*;
import net.sourceforge.vmb.view.*;

import org.apache.log4j.*;

public class DeviceLauncher {

    private static final String PROPERTYFILE = "vmb.properties";
    protected static Logger logger = Logger.getLogger(DeviceLauncher.class);


	/**
	 * @param args
	 */
	public static void main(String[] args) {
        PropertyManager propertyManager = new PropertyManager(PROPERTYFILE);

        final FrameBufferDevice deviceWindow = new FrameBufferDevice(propertyManager.getWidth(), 
                propertyManager.getHeight());
        final MotherboardConnection connection = new MotherboardConnection(propertyManager.getStartAddress(), 
                4 * propertyManager.getWidth() * propertyManager.getHeight());
        connection.addConnectionListener(deviceWindow);
        connection.addConnectionListener(new LoggingConnectionListener());
		
		deviceWindow.setVisible(true);
		
		try {
			connection.establishConnection(InetAddress.getByName(propertyManager.getHostname()), 
			        propertyManager.getPort());
		} catch (UnknownHostException e) {
			logger.error("Could not connect to host " + propertyManager.getHostname(), e);
			JOptionPane.showMessageDialog(deviceWindow, "The host " + propertyManager.getHostname() + " is unknown",  "Error", JOptionPane.ERROR_MESSAGE);
			deviceWindow.dispose();
			return;
		} catch (IOException e) {
            logger.error("Could not connect to host " + propertyManager.getHostname(), e);
            JOptionPane.showMessageDialog(deviceWindow, "Could not connect to host " + propertyManager.getHostname(),  "Error", JOptionPane.ERROR_MESSAGE);
            deviceWindow.dispose();
            return;
		}

		deviceWindow.addWindowListener(new WindowListener(){
			public void windowOpened(WindowEvent arg0) {
			}
			public void windowClosing(WindowEvent arg0) {
                try {
                    connection.shutDown();
                    deviceWindow.dispose();
                } catch (IOException e) {
                    logger.error("Exception during shutdown.", e);
                }
				return;
			}
			public void windowClosed(WindowEvent arg0) {
                return;
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



}
