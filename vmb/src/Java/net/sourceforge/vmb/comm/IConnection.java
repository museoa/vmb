/*
 * Created on 25.01.2008
 *
 */
package net.sourceforge.vmb.comm;

import java.io.*;
import java.net.*;

public interface IConnection {

    public abstract void establishConnection(InetAddress address, int port)
            throws IOException;

    public abstract byte readByte() throws IOException;

    public abstract void addConnectionListener(IConnectionListener listener);

    public abstract void shutDown() throws IOException;

}