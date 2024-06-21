import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.AffineTransform;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;
import javax.swing.JPanel;
import javax.swing.Timer;

/**
 * Subclass of JPanel designed to provide graphics for the simulation
 */
public class Map extends JPanel {
    //random number generator
    private Random random = new Random();

    private TrailPanel trailPanel;   //drawn map

    //colors
    public static final Color OBSTACLE_COLOR = new Color(84, 15, 9);
    private static final Color ROBOT_COLOR = new Color(38, 35, 35);

    //list of robots
    public List<Robot> robots;

    //cursor coordinates
    private int cursorX;
    private int cursorY;

    //List of different types of obstacles
    private List<Obstacle> hills;
    private List<Obstacle> rocks;
    private List<Obstacle> walls; 
    private List<Obstacle> cliffs; 

    //boolean for checking for collisions
    public boolean collided = false;

    //variables for moving backwards
    double backX;
    double backY;
    int backSteps = 0;

    /**
     * Allows a robot to check if an obstacle is detected in front of it.
     * 
     * @param rob the robot
     * @return a set containing coordinate information about all points where an obstacle is found
     */
    public Set<ScannedPoint> scanForObstacles(Robot rob) {
        Set<ScannedPoint> detectedPoints = new HashSet<>();
        //for (double angle = 0; angle < 2 * Math.PI; angle += Math.PI / Robot.SENSOR_RESOLUTION) {
            for (int r = 0; r < Robot.SENSOR_RANGE; r += 5) {
                int scanX = (int) (rob.x() + r * Math.cos(rob.getAngle()));
                int scanY = (int) (rob.y() + r * Math.sin(rob.getAngle()));
                if (collisionAt(scanX, scanY)) {
                    Color color = collidesWith(rocks, scanX, scanY) ? Color.red : Color.black;
                    detectedPoints.add(
                        new ScannedPoint(
                            scanX + random.nextInt(-10, 10),
                            scanY + random.nextInt(-10, 10),
                            color
                        )
                    );
                    break;
                }
            }
        //}

        return detectedPoints;
    }
    
    /**
     * Updates cursor coordinates to the specified point
     * @param x the x coordinate of the new point
     * @param y the y coordinate of the new point
     */
    public void moveCursorAt(int x, int y) {
        cursorX = x;
        cursorY = y;
    }
    
    /**
     * Updates the orientation of a robot
     * @param rob the robot
     */
    public void updateRotation(Robot rob) {
        double dx = cursorX - rob.x();
        double dy = cursorY - rob.y();

        rob.setTarget(Math.atan2(dy, dx));
        repaint();
    }

    //timer used for repeatedly moving back
    public volatile Timer moveBack = new Timer(30, new ActionListener() {
        /**
         * Allows a robot to move backwards
         * @param rob the robot
         */
        public void stepBack(Robot rob) {
            //move the robot backwards until 17 steps are reached or until a collision is inevitable
            if (backSteps < 17 && canMoveTo(rob.x() + (int) backX, rob.y() + (int) backY)) {
                moveBack(rob);
                rob.setAngle(rob.getAngle() + rob.rotationFactor);
                backSteps++;
                rob.setLocation(rob.x() + backX, rob.y() + backY);
                trailPanel.addPoint(new Point(rob.x() + random.nextInt(-10, 10), rob.y() + random.nextInt(-10, 10)));
                repaint();
            } else {
            //resume moving forward
                backSteps = 0;
                rob.movingBackwards = false;
                rob.rotationFactor = random.nextBoolean() ? -rob.rotationFactor : rob.rotationFactor;
                for (Robot r : robots) {
                    startWiggle(r);
                }
            }
        }
        public void actionPerformed(ActionEvent e) {
            for (Robot robot : robots) {
                if (robot.movingBackwards) {
                    stepBack(robot);
                }
            }
        }
    });

    //timer for repeating forward movement towards the cursor
    public volatile Timer move = new Timer(15, new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            moveRobotTowardsCursor();
        }
    });

    //timer for repeating automated movement
    public volatile Timer autoMove = new Timer(30, new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            for (Robot robot : robots) {
                if (!robot.movingBackwards) {
                    explore(robot);
                }
            }
        }
    });

    /**
     * Startes the wiggle timer of a robot
     * @param rob the robot
     */
    public void startWiggle(Robot rob) {
        rob.setAngle(rob.getAngle() - rob.rotationFactor / 5 / 2);
        rob.wiggle.start();
    }

    /**
     * Allows the robot to explore the map by taking a single step in a computed direction
     * @param rob the robot
     */
    public void explore(Robot rob) {
        double speed = 3; // Adjust the speed as needed
        Set<ScannedPoint> sensorData = scanForObstacles(rob);
    
        // Check if obstacles are detected
        boolean obstacleDetected = false;
        for (ScannedPoint p : sensorData) {
            if (p.getColor().equals(Color.black) || p.getColor().equals(Color.red)) {
                trailPanel.addObstacle(p, p.getColor());
                obstacleDetected = true;
                break;
            }
        }
    
        // If an obstacle is detected, find a direction with the most clearance
        if (obstacleDetected) {
            double bestAngle = rob.getAngle();
            int maxClearance = 0;
    
            for (double angle = 0; angle < 2 * Math.PI; angle += Math.PI / 8) { // Check multiple directions
                int clearance = calculateClearance(rob.x(), rob.y(), angle);
                if (clearance > maxClearance) {
                    maxClearance = clearance;
                    bestAngle = angle;
                }
            }
            rob.setAngle(bestAngle > rob.getAngle() ? rob.getAngle() + rob.turnFactor : rob.getAngle() - rob.turnFactor);
        } else {
            rob.setAngle(rob.getAngle() + rob.rotationFactor / 5);
        }
    
        double moveX = speed * Math.cos(rob.getAngle());
        double moveY = speed * Math.sin(rob.getAngle());
    
        if (canMoveTo((int) (rob.x() + moveX), (int) (rob.y() + moveY))) {
            rob.setLocation(rob.x() + moveX, rob.y() + moveY);
            trailPanel.addPoint(new Point(rob.x() + random.nextInt(-10, 10), rob.y() + random.nextInt(-10, 10))); // Add point to trail
        } else {
            backX = -moveX;
            backY = -moveY;
            rob.movingBackwards = true;
            moveBack(rob);
        }
        repaint();
    }
    
    /**
     * Calculates the clearance in a given direction
     * @param x the x coordinate of the center of the checked area
     * @param y the y coordinate of the center of the checked area
     * @param angle the orientation angle
     * @return the computed clearance
     */
    private int calculateClearance(int x, int y, double angle) {
        int clearance = 0;
        for (int r = 0; r < Robot.SENSOR_RANGE; r += 5) {
            int scanX = (int) (x + r * Math.cos(angle));
            int scanY = (int) (y + r * Math.sin(angle));
            if (collisionAt(scanX, scanY)) {
                break;
            }
            clearance++;
        }
        return clearance;
    }

    /**
     * Initates the back movement of a robot
     * @param rob the robot
     */
    public void moveBack(Robot rob) {
        rob.wiggle.stop();
        moveBack.start();
    }

    /**
     * Clears all data
     */
    public void clearAll() {
        this.robots.clear();
        this.rocks.clear();
        this.hills.clear();
        this.walls.clear();
        this.cliffs.clear();
        this.trailPanel.clearAll();
    }

    /**
     * Initializes a map with information about obstacle, robots and the drawn map
     * 
     * @param rox a list of coordinate information about rocks
     * @param hs a list of coordinate information about hills
     * @param ws a list of coordinate information about walls
     * @param cs a list of coordinate information about cliffs
     * @param robs a list of coordinate information about robots
     * @param tp the drawn map
     */
    public Map(List<Obstacle> rox, List<Obstacle> hs, List<Obstacle> ws, List<Obstacle> cs, List<Robot> robs, TrailPanel tp) {
        this.robots = new ArrayList<>(robs);
        this.rocks = new ArrayList<>(rox);
        this.hills = new ArrayList<>(hs);
        this.walls = new ArrayList<>(ws);
        this.cliffs = new ArrayList<>(cs);
        this.trailPanel = tp;
        
        for (Robot robot : robots) {
            startWiggle(robot);
        }

        addMouseMotionListener(new MouseAdapter() {
            /**
             * Updates cursor coordinates in response to a mouse event
             * @param e the mouse event
             */
            private void updateCursor(MouseEvent e) {
                cursorX = e.getX();
                cursorY = e.getY();
            }

            @Override
            public void mouseMoved(MouseEvent e) {
                updateCursor(e);
                updateRotation(robots.get(0));
            }

            @Override
            public void mouseDragged(MouseEvent e) {
                updateCursor(e);
                updateRotation(robots.get(0));
            }
        });

        addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                move.start();
            }

            @Override
            public void mouseReleased(MouseEvent e) {
                move.stop();
            }
        });
    }

    /**
     * Draws an obstacle on the map.
     * @param g the graphics tool
     * @param rect the rectangle representing the obstacle
     * @param colors a list of colors to choose from
     * @param index the index of the first desired color from the list
     */
    private void placeObject(Graphics g, Obstacle rect, Color[] colors, int index) {
        g.setColor(colors[index % colors.length]);
        try {
            if (rect.isVisible()) {
                if (rect.isCliff()) {
                    g.fillOval(rect.x, rect.y, rect.getDiameter(), rect.getDiameter());
                } else {
                    g.fillRect(rect.x, rect.y, rect.width, rect.height);
                }
            }
        } catch (NullPointerException e) {
            return;
        } catch (ArrayIndexOutOfBoundsException e) {
            return;
        }
    }

    /**
     * Draws rocks on the map.
     * @param g the graphics tool
     */
    private void placeRocks(Graphics g) {
        int rng = 121088;

        for (Obstacle rock : rocks) {
            placeObject(g, rock, 
                new Color[] {
                    Color.red, Color.green, Color.blue, Color.white, Color.black
                },
                rng
            );

            rng = rng % 2 == 0 ? rng + 3 : rng - 7;
        }
    }

    /**
     * Draws obstacles on the map.
     * 
     * @param g the graphics tool
     */
    private void placeObstacles(Graphics g) {
        for (Obstacle obstacle : hills) {
            placeObject(g, obstacle, new Color[] {OBSTACLE_COLOR}, 0);
        }
        for (Obstacle cliff : cliffs) {
            placeObject(g, cliff, new Color[] {Color.black}, 0);
        }
        for (Obstacle wall : walls) {
            placeObject(g, wall, new Color[] {Color.black}, 0);
        }
    }

    /**
     * Fills the map with all required elements
     * @param g the graphics component
     */
    private void randomizeMap(Graphics g) {
        placeObstacles(g);
        placeRocks(g);
    }

    /**
     * Moves the robot one step towards the point where the mouse is located
     */
    public void moveRobotTowardsCursor() {
        double dx = cursorX - robots.get(0).x();
        double dy = cursorY - robots.get(0).y();
        double distance = Math.sqrt(Math.abs(dx * dx) + Math.abs(dy * dy));
        double speed = 3; // Adjust the speed as needed
        double moveX = (speed / distance) * dx;
        double moveY = (speed / distance) * dy;

        Set<ScannedPoint> sensorData = scanForObstacles(robots.get(0));
        for (ScannedPoint p : sensorData) {
            trailPanel.addObstacle(p, p.getColor()); // Add detected obstacles in red color
        }

        if (canMoveTo(
                    (int) (robots.get(0).x() + moveX),
                    (int) (robots.get(0).y() + moveY))
                && (robots.get(0).x() != cursorX || robots.get(0).y() != cursorY)
            ) {
            robots.get(0).setLocation(robots.get(0).x() + moveX, robots.get(0).y() + moveY);
        } else if (move.isRunning()) {
            move.stop();
        }
        repaint();
    }

    /**
     * Checks whether the robot is can move towards a point without causing collision
     * @param x the x coordinate of the destination point of the robot
     * @param y the y coordinate of the destination point of the robot
     * @return true if the robot can safely move towards the destination, false otherwise
     */
    public boolean canMoveTo(int x, int y) {
        // Check if the robot can move to the specified location without colliding with obstacles
        for (int i = 0; i < Robot.WIDTH; i++) {
            for (int j = 0; j < Robot.HEIGHT; j++) {
                int newX = x - Robot.WIDTH / 2 + i;
                int newY = y - Robot.HEIGHT / 2 + j;
                if (collisionAt(newX, newY)) {
                    return false; // Collision detected
                }
            }
        }
        return true; // No collision detected
    }
    
    /**
     * Checks if a robot can rotate without causing a collision
     * @param rob the robot
     * @return true if the robot can safely rotate, false otherwise
     */
    private boolean canRotate(Robot rob) {
        // Check if the Obstacle can rotate without colliding with the brown-painted object
        for (int i = 0; i < Robot.WIDTH; i++) {
            for (int j = 0; j < Robot.HEIGHT; j++) {
                int newX = rob.x() + (int) ((i - Robot.WIDTH / 2) * Math.cos(rob.getAngle()) - (j - Robot.HEIGHT / 2) * Math.sin(rob.getAngle()));
                int newY = rob.y() + (int) ((i - Robot.WIDTH / 2) * Math.sin(rob.getAngle()) + (j - Robot.HEIGHT / 2) * Math.cos(rob.getAngle()));
                if (collisionAt(newX, newY)) {
                    return false; // Collision detected
                }
            }
        }
        return true; // No collision detected
    }

    /*private boolean collidesWithRobot(Robot rob) {
        for (Robot r : robots) {
            if (rob.equals(r)) {
                continue;
            }
            //-Robot.WIDTH / 2, -Robot.HEIGHT / 2, Robot.WIDTH, Robot.HEIGHT
            if (rob.x() - Robot.WIDTH / 2 >= r.x() - Robot.WIDTH / 2 
                && rob.x() - Robot.WIDTH / 2 <= r.x() + Robot.WIDTH
                && rob.y() - Robot.HEIGHT / 2 >= r.y() - Robot.HEIGHT / 2
                && rob.y() - Robot.HEIGHT / 2 <= r.y() + Robot.HEIGHT) {
                return true;
            }
        }
        return false;
    }*/
    
    /**
     * Checks if a specified point collides with at least one of many given obstacles
     * 
     * @param obs the collection of obstacles
     * @param x the x coordinate of a point
     * @param y the y coordinate of a point
     * @return true if a collision occurs, false otherwise
     */
    private boolean collidesWith(List<Obstacle> obs, int x, int y) {
        for (Obstacle o : obs) {
            if (x >= o.x && x <= o.x + o.width && y >= o.y && y <= o.y + o.height) {
                o.setVisible(true);
                return true;
            }
        }

        return false;
    }

    /**
     * Checks if a collision occurs at a specified point
     * 
     * @param x the x coordinate of the given point
     * @param y the y coordinate of the given point
     * @return true if a collision occurs, false otherwise
     */
    private boolean collisionAt(int x, int y) {
        return collidesWith(hills, x, y) 
            || collidesWith(rocks, x, y) 
            || collidesWith(walls, x, y) 
            || collidesWith(cliffs, x, y);
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2d = (Graphics2D) g;

        //draw all obstacles
        g2d.setColor(OBSTACLE_COLOR);
        randomizeMap(g);    

        //draw the robots
        for (Robot robot : robots) {
            AffineTransform oldTransform = g2d.getTransform(); // Save the current transform
            // Draw the rotated Obstacle
            g2d.setColor(ROBOT_COLOR);
            g2d.translate(robot.x(), robot.y()); // Translate to the center of rotation
            if (canRotate(robot)) {
                g2d.rotate(robot.getAngle()); // Rotate by the angle
            }
            while (!robot.adjustedInitialPosition && !canRotate(robot)) {
                robot.setLocation(robot.x() + 1, robot.y() + 1);
            }

            if (!robot.adjustedInitialPosition) {
                robot.adjustedInitialPosition = true;
            }
            g2d.fillRect(-Robot.WIDTH / 2, -Robot.HEIGHT / 2, Robot.WIDTH, Robot.HEIGHT); // Draw the Obstacle centered at (0, 0)
            g2d.setTransform(oldTransform); // Restore the original transform
        }
    }  
}
