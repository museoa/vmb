package edu.fhm.mmixmb.view;

import java.awt.*;
import java.awt.image.*;

import edu.fhm.mmixmb.comm.*;

public class FrameBufferDevice extends Frame implements IConnectionListener{

	private Image img;
	private Insets insets;

	public FrameBufferDevice(){
		initComponents();
	}

	private void initComponents() {
		this.setLayout(new GridLayout(3, 1));

		pack();

	}

	/** Darstellung eines Farb-Bildes im Fenster.
	 * @param grid Zweidimensionales int-Array mit Pixeldaten.
	 * Jedes Element liefert einen RGB-Wert im Java-Farbformat
	 * (xxxxxxxx.rrrrrrrr.gggggggg.bbbbbbbb).
	 * Der Alphakanal wird immer auf 255 gestellt.
	 */
	public void display(int[][] grid)
	{
		int w = grid.length;
		int h = grid[0].length;

		int[] pixel = new int[w*h];
		int p = 0;
		for(int y = h - 1;  y >= 0;  y--)
			for(int x = 0;	x < w;	x++)
				pixel[p++] = 0xFF000000 | grid[x][y];

		load(w, h, pixel);
	}

	/** Uebertraegt die Pixeldaten aus dem Array in ein neues
	 * Image-Objekt und zeigt dieses im Fenster.
	 */
	private void load(int w, int h, int[] pixel)
	{
		img = createImage(new MemoryImageSource(w, h, pixel, 0, w));

		insets = getInsets();
		setSize(w, h);

		repaint();
	}

	/** Callbackmethode zum Neuzeichnen des Fensters.
	 * @param g Graphics context.
	 */
	public void paint(Graphics g)
	{
		g.drawImage(img, insets.left, insets.top, this);
	}

	/** Callbackmethode zum Neuzeichnen des Fensters.
	 * @param g Graphics context.
	 */
	public void update(Graphics g)
	{
		paint(g);
	}

	public void DataReceived() {
		// TODO Auto-generated method stub
		
	}

	public void powerOff() {
		// TODO Auto-generated method stub
		
	}

	public void powerOn() {
		// TODO Auto-generated method stub
		
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
}
