/*
 * Created on 01.09.2007
 *
 * TODO add description
 */
package net.sourceforge.vmb.comm;

import org.apache.log4j.*;

public class LoggingConnectionListener implements IConnectionListener {

    private final static int DATALINELENGTH = 10; 
    protected Logger logger = Logger.getLogger(LoggingConnectionListener.class);
    
    public LoggingConnectionListener() {
        PropertyConfigurator.configure("log4j.properties");
    }

    public void dataReceived(int offset, byte[] payload) {
        logger.info("Data received: Offset = " + Integer.toHexString(offset));
        StringBuilder line = new StringBuilder("   ");//3 characters Vorlauf
        for (int i = 0; i < payload.length; i++) {
            line.append(Integer.toHexString(((int)payload[i]) & 0xFF));
            line.append(' ');
            if(i % DATALINELENGTH == 0) {
                logger.info(line);
                line.delete(3, line.length() - 1);
            }
        }
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
}
