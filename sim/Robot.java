import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.Timer;

/**
 * Collection of data representing a simulated robot.
 */
public class Robot {
    //global public constants
    public static final int WIDTH = 25; // Width of the Obstacle
    public static final int HEIGHT = 50; // Height of the Obstacle
    public static final int SENSOR_RANGE = 160;
    public static final int SENSOR_RESOLUTION = 10;

    //global private variables
    private double angle; // Initial angle of rotation
    private double target;  //the desired angle at which the robot should turn
    private int centerX = 100; // X-coordinate of the center of the robot
    private int centerY = 100; // Y-coordinate of the center of the robot

    //factor at which angle increases such that robot rotates to simulate the wiggle movement
    public double rotationFactor = .1;

    //factor at which angle increases such that the robot performs a turn
    public double turnFactor = .2;

    //public global booleans
    public boolean movingBackwards = false;
    public boolean adjustedInitialPosition;

    //timer which flips the rotation factor repeatedly to achieve the wiggle movement
    public volatile Timer wiggle = new Timer(500, new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            rotationFactor*= -1;
        }
    });   

    //default constructor
    public Robot() {}

    /**
     * Initializes location data for a Robot object.
     * 
     * @param x the x coordinate of the center of the robot
     * @param y the y coordinate of the center of the robot
     */
    public Robot(int x, int y) {
        this.centerX = x;
        this.centerY = y;
    }

    /**
     * Initializes location data for a Robot object along with
     * an initial angle.
     * 
     * @param x the x coordinate of the center of the robot
     * @param y the y coordinate of the center of the robot
     * @param a the value of the rotation angle of the robot
     */
    public Robot(int x, int y, double a) {
        this.centerX = x;
        this.centerY = y;
        this.angle = a;
    }

    //getters
    public int x() {
        return this.centerX;
    }

    public int y() {
        return this.centerY;
    }

    public double getAngle() {
        return this.angle;
    }

    //setters
    public void setTarget(double t) {
        this.target = t;
    }

    public void setAngle(double a) {
        this.angle = a;
    }

    public void setLocation(int x, int y) {
        this.centerX = x;
        this.centerY = y;
    }

    public void setLocation(double x, double y) {
        this.centerX = (int) x;
        this.centerY = (int) y;
    }

    /**
     * Checks if the robot reached its turning goal.
     * 
     * @return true if the angle has reached the target, false otherwise
     */
    public boolean reachedTarget() {
        return Math.abs(this.target) >= Math.abs(this.angle);
    }

    /**
     * increments the rotation angle;
     */
    public void increaseAngle() {
        this.angle = this.angle < this.target ? this.angle + 0.1 : this.angle - 0.1;
    }

    /**
     * Returns the coordinates (x, y) of the current location of the robot
     * 
     * @return the point where the robot is currently located
     */
    public Point getRobotPosition() {
        return new Point(centerX, centerY);
    }
}