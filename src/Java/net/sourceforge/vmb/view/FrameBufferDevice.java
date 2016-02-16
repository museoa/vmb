package net.sourceforge.vmb.view;

import java.awt.*;

import javax.swing.*;

import org.apache.log4j.*;

import net.sourceforge.vmb.comm.*;

public class FrameBufferDevice extends JFrame implements IConnectionListener{

    private static final long serialVersionUID = -4947407240818597820L;
    private final int width;
    private final int height;
    private int bitmap[][];
    private DeviceCanvas canvas;

    protected Logger logger = Logger.getLogger(FrameBufferDevice.class);
    
	public FrameBufferDevice(int width, int height){
        this.height = height;
        this.width = width;
        bitmap = new int[width][height];
        PropertyConfigurator.configure("log4j.properties");
		initComponents();
	}

	private void initComponents() {
        canvas = new DeviceCanvas(width, height);
        add(canvas);
        setSize(width, height);
        setPreferredSize(new Dimension(width, height));
        this.setResizable(false);

		pack();
	}

	/** Callbackmethode zum Neuzeichnen des Fensters.
	 * @param g Graphics context.
	 */
	public void update(Graphics g)
	{
		repaint();
	}

	public void dataReceived(int offset, byte[] payload) {
		offset >>= 2;
        final int x = offset % width;
        final int y = offset / width;
        if(x >= width || y >= height){
        	logger.fatal("Coordinates out of bound: " + x + " " + y);
        	return;
        }
        final int color = (payload[3] & 0xff) | ((payload[2] & 0xff) << 8) | ((payload[1] & 0xff) << 16);
        bitmap[x][y] = color;
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                canvas.drawPixel(canvas.getGraphics(), x, y);
            }});
	}

	public void powerOff() {
	}

	public void powerOn() {
	}

	public void interruptRequest(int irqNumber) {
	}

	public void readRequest() {
	}

	public void reset() {
	}

	public void writeRequest() {
	}

    public void clear() {
    }
    
    public void terminate() {
        EventQueue.invokeLater(new Runnable() {
            public void run() {
                FrameBufferDevice.this.setVisible(false);
                FrameBufferDevice.this.dispose();
            }});
    }
    
    private class DeviceCanvas extends Canvas{

        private static final long serialVersionUID = -1746762048977413579L;
        private final int width;
        private final int height;

        public DeviceCanvas(int width, int height) {
            this.width = width;
            this.height = height;
        }

        public void paint(Graphics g) {
            g.setPaintMode();
            for(int w = 0; w < width; w++) {
                for(int h = 0; h< height; h++) {
                    drawPixel(g, w, h);
                }
            }
        }

        private void drawPixel(Graphics g, int w, int h) {
            Color color = new Color((bitmap[w][h] >> 16) & 0xff, (bitmap[w][h] >> 8) & 0xff, bitmap[w][h] & 0xff);
            g.setColor(color);
            g.drawLine(w, h, w, h);
        }
        
    }
}
