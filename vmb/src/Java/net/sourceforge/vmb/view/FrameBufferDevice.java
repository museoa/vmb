package net.sourceforge.vmb.view;

import java.awt.*;

import javax.swing.*;

import net.sourceforge.vmb.comm.*;

public class FrameBufferDevice extends Frame implements IConnectionListener{

    private JPanel drawingArea;
    private final int width;
    private final int height;
    private int bitmap[][];

	public FrameBufferDevice(int width, int height){
        this.height = height;
        this.width = width;
        bitmap = new int[width][height];
		initComponents();
	}

	private void initComponents() {
        this.setResizable(false);
        drawingArea = new JPanel();
        this.add(drawingArea);

        drawingArea.setPreferredSize(new Dimension(width, height));

		pack();
	}

    public void display(long address, int pixel) {
//        bitmap[][] = pixel;
        drawingArea.getGraphics().setColor(Color.GREEN);
        drawingArea.getGraphics().drawRect(10, 10, 1, 1);
    }

	/** Callbackmethode zum Neuzeichnen des Fensters.
	 * @param g Graphics context.
	 */
	public void paint(Graphics g)
	{
        System.out.println("paint");
        if(bitmap == null) {
            return;
        }

        for(int w = 0; w < width; w++) {
            for(int h = 0; h< height; h++) {
                    drawPixel(w, h);
            }
        }
	}

	public void drawPixel(int x, int y) {
	    if(bitmap[x][y] != 0) {
	        drawingArea.getGraphics().setColor(new Color(bitmap[x][y]));
	        drawingArea.getGraphics().drawRect(x, y, 1, 1);
	    }
    }

	/** Callbackmethode zum Neuzeichnen des Fensters.
	 * @param g Graphics context.
	 */
	public void update(Graphics g)
	{
        System.out.println("update");
		repaint();
	}

	public void DataReceived(int offset, byte[] payload) {
        final int w = offset / width;
        final int h = offset % width;
        int color = payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24);
        bitmap[w][h] = color;
		SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                drawPixel(w, h);
            }});
	}

	public void powerOff() {
		// TODO Auto-generated method stub

	}

	public void powerOn() {
        bitmap = new int[width][height];
	}

	public void interruptRequest(int irqNumber) {
		// TODO Auto-generated method stub

	}

	public void readRequest() {
		// TODO Auto-generated method stub

	}

	public void reset() {
		// TODO Auto-generated method stub

	}

	public void writeRequest() {
		// TODO Auto-generated method stub

	}

    public void clear() {
        // TODO Auto-generated method stub

    }
}
