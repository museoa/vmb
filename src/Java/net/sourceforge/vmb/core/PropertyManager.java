/*
 * Created on 03.03.2008
 *
 */
package net.sourceforge.vmb.core;

import java.io.*;
import java.util.*;

import net.sourceforge.vmb.view.*;

import org.apache.log4j.*;

public class PropertyManager {
    
    private int port = 9002;
    private String hostName = "localhost";
    private long startAddress = 0x2000000000000l;
    private int width = 640;
    private int height = 480;
    private Properties properties;
    
    protected Logger logger = Logger.getLogger(FrameBufferDevice.class);
    
    public PropertyManager(String fileName) {
        properties = new Properties();
        try {
            properties.load(new FileInputStream(fileName));
            hostName = properties.getProperty("host");
            port = getInt("port", port);
            width = getInt("width", width);
            height = getInt("height", height);
            startAddress = getLong("startaddress", startAddress);
        } catch (FileNotFoundException e1) {
            logger.warn("property-file " + fileName + " not found; using defaults.");
        } catch (IOException e1) {
            logger.warn("property-file " + fileName + " could not be opened; using defaults.", e1);
        } catch(NumberFormatException e) {
            logger.warn("property-file contained illegal port specification: " + properties.getProperty("port"));
        }
    }
    
    private int getInt(String property, int defaultValue) {
        try {
            return Integer.parseInt(properties.getProperty(property));
        } catch(NumberFormatException e) {
            logger.warn("property-file contained illegal integer specification for " + property + ": "+ properties.getProperty(property) + " - using default " + defaultValue);
        }
        return defaultValue;
    }

    private long getLong(String property, long defaultValue) {
        try {
            return Long.parseLong(properties.getProperty(property), 16);
        } catch(NumberFormatException e) {
            logger.warn("property-file contained illegal integer specification for " + property + ": "+ properties.getProperty(property) + " - using default " + defaultValue);
        }
        return defaultValue;
    }

    /**
     * @return the port
     */
    public int getPort() {
        return port;
    }

    /**
     * @param port the port to set
     */
    public void setPort(int port) {
        this.port = port;
    }

    /**
     * @return the host
     */
    public String getHostname() {
        return hostName;
    }

    /**
     * @return the startAddress
     */
    public long getStartAddress() {
        return startAddress;
    }

    /**
     * @param startAddress the startAddress to set
     */
    public void setStartAddress(long startAddress) {
        this.startAddress = startAddress;
    }

    /**
     * @return the hostName
     */
    public String getHostName() {
        return hostName;
    }

    /**
     * @param hostName the hostName to set
     */
    public void setHostName(String hostName) {
        this.hostName = hostName;
    }

    /**
     * @return the width
     */
    public int getWidth() {
        return width;
    }

    /**
     * @param width the width to set
     */
    public void setWidth(int width) {
        this.width = width;
    }

    /**
     * @return the height
     */
    public int getHeight() {
        return height;
    }

    /**
     * @param height the height to set
     */
    public void setHeight(int height) {
        this.height = height;
    }

}
