/*
 * Created on 01.09.2007
 *
 * TODO add description
 */
package net.sourceforge.vmb.comm;

import org.apache.log4j.*;

public class LoggingConnectionListener implements IConnectionListener {

    protected Logger logger = Logger.getLogger(LoggingConnectionListener.class);

    public LoggingConnectionListener() {
        PropertyConfigurator.configure("log4j.properties");
    }

    /**
     * to hex converter
     */
    private static final char[] toHex = {
        '0', '1', '2', '3', '4', '5', '6',
        '7', '8', '9', 'a', 'b', 'c', 'd',
        'e', 'f'};

    /**
     * Converts b[] to hex string.
     * @param b the byte array to convert
     * @return a Hex representation of b.
     */
    private static String toHexString(byte[] b)
    {
        int pos = 0;
        char[] c = new char[b.length * 3];
        for (int i = 0; i < b.length; i++) {
            c[pos++] = toHex[ (b[i] >> 4) & 0x0F];
            c[pos++] = toHex[b[i] & 0x0f];
            c[pos++] = ' ';
        }
        return new String(c);
    }     

    public void dataReceived(int offset, byte[] payload) {
        logger.info("Data received: Offset = " + Integer.toHexString(offset) 
                + "   " + toHexString(payload));
    }

    public void interruptRequest(int irqNumber) {
        logger.info("Irq " + irqNumber);
    }

    public void powerOff() {
        logger.info("Power Off");
    }

    public void powerOn() {
        logger.info("Power On");
    }

    public void readRequest() {
        logger.info("Read");
    }

    public void reset() {
        logger.info("Reset");
    }

    public void writeRequest() {
        logger.info("Write");
    }
    public void terminate() {
        logger.info("Terminate");
    }
    
}
