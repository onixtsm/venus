import java.awt.Color;
import java.awt.Point;

/**
 * Subclass of point containing additional information about the color 
 * of a pixel located at a specified point.
 */
public class ScannedPoint extends Point {
    private Color color;

    //default constructor
    ScannedPoint() {
        super();
    }

    /**
     * Initializes a scanned point using only coordinate information
     * 
     * @param x the x coordinate
     * @param y the y coordinate
     */
    ScannedPoint(int x, int y) {
        super(x, y);
    }

    /**
     * Initializes a scanned point using coordinate and color information
     * 
     * @param x the x coordinate
     * @param y the y coordinate
     * @param c the color
     */
    ScannedPoint(int x, int y, Color c) {
        super(x, y);
        this.color = c;
    }

    //getter method
    public Color getColor() {
        return this.color;
    }

    //setter method
    public void setColor(Color c) {
        this.color = c;
    }
}