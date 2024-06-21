import java.awt.Rectangle;
import java.util.Collection;

/**
 * Subclass of rectangle representing a simulated obstacle
 */
public class Obstacle extends Rectangle {
    //boolean used for checking if the obstacle is visible
    private boolean visible = false;   

    //diameter variable for circular obstacles
    private int diameter = -1;

    //default constructor
    public Obstacle() {}

    /**
     * Creates a rectangular obstacle with specified dimensions and location,
     * using derived attributes of the superclass.
     * 
     * @param x the x coordinate of the left corner of the rectangle
     * @param y the y coordinate of the left corner of the rectangle
     * @param width the width of the rectangle
     * @param height the height of the rectangle
     */
    public Obstacle(int x, int y, int width, int height) {
        super(x, y, width, height);
    }

    /**
     * Creates a circular obstacle with specified dimensions and location.
     * 
     * @param x the x coordinate of the left corner of the circle's circumscribed rectangle
     * @param y the y coordinate of the left corner of the circle's circumscribed rectangle
     * @param d the diameter of the circle
     */
    public Obstacle(int x, int y, int d) {
        this.x = x;
        this.y = y;
        this.width = d;
        this.height = d;
        this.diameter = d;
    }

    //setters
    public void setVisible(boolean v) {
        this.visible = v;
    }

    //getters
    public int getDiameter() {
        return this.diameter;
    }

    /**
     * Checks if an obstacle is visible or not.
     * 
     * @return true if an obstacle is visible, false otherwise.
     */
    public boolean isVisible() {
        return this.visible;
    }

    /**
     * Checks if an obstacle is a cliff, i.e. whether it has a diameter
     * and is therefore circular.
     * 
     * @return true if the obstacle is a cliff, false otherwise
     */
    public boolean isCliff() {
        return this.diameter != -1;
    }

    /**
     * Checks if an obstacle collides with another obstacle.
     * 
     * @param o the other obstacle
     * @return true if a collision occurs, false otherwise
     */
    public boolean collidesWith(Obstacle o) {
        if (this.getMaxX() < o.getMinX() || this.getMinX() > o.getMaxX()) {
            return false; // No horizontal overlap
        } else if (this.getMaxY() < o.getMinY() || this.getMinY() > o.getMaxY()) {
            return false; // No vertical overlap
        } else if (o.contains(this)) {
            return true; // A is completely contained within B
        } else if (this.contains(o)) {
            return true; // B is completely contained within A
        } else {
            return true; // Partial overlap
        }
    }

    /**
     * Checks if an obstacle is close to another obstacle.
     * 
     * @param o the other obstacle
     * @return true if the obstacle is close to another, false otherwise
     */
    public boolean isCloseTo(Obstacle o) {
        final Obstacle temp = new Obstacle(o.x - 60, o.y - 60, o.width + 120, o.height + 120);
        return this.collidesWith(temp);
    }

    /**
     * Checks if an obstacle collides with at least one obstacle from a collection of obstacles.
     * 
     * @param obs the collection of obstacles
     * @return true if a collision occurs, false otherwise
     */
    public boolean collidesWith(Collection<Obstacle> obs) {
        for (Obstacle ob : obs) {
            if (this.collidesWith(ob)) {
                return true;
            }
        }

        return false;
    }

    /**
     * Checks if an obstacle is close to at least one obstacle from a collection of obstacles
     * 
     * @param obs the collection of obstacles
     * @return true if an obstacle is close to at least one obstacle from {obs}, false otherwise
     */
    public boolean isCloseTo(Collection<Obstacle> obs) {
        for (Obstacle ob : obs) {
            if (this.isCloseTo(ob)) {
                return true;
            }
        }

        return false;
    }
}
