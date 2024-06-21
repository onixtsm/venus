import java.awt.Color;
import java.awt.KeyEventDispatcher;
import java.awt.KeyboardFocusManager;
import java.awt.Point;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;

/**
 * Simulation of the 2 robots exploring a rectangular area of the surface of Venus
 */
public class Simulation {
    //enumeration for different obstacle types
    private enum Obstacles {ROCK, HILL, CLIFF};
    
    //window dimension
    private static final int WIDTH = 800;
    private static final int HEIGHT = 500;
    private static final int WALL_THICKNESS = 20;

    //GUI elements
    private JFrame camera;
    private Map map;
    private TrailPanel trailPanel;

    //lists of different kinds of obstacles
    private List<Obstacle> hills = new ArrayList<>();
    private List<Obstacle> rocks = new ArrayList<>();
    private List<Obstacle> walls = new ArrayList<>();
    private List<Obstacle> cliffs = new ArrayList<>();

    //boolean for toggling obstacle visibility
    private boolean madeVisible = false;

    //colors
    private static final Color GND_COLOR = new Color(117, 71, 55);
    
    //random data generator
    private Random random = new Random();

    //spawn point of the first robot
    private Point spawn = new Point(
        random.nextInt(WALL_THICKNESS + 1, WIDTH - WALL_THICKNESS - 1),
        random.nextInt(WALL_THICKNESS + 1, HEIGHT - WALL_THICKNESS - 1)
    );

    //checks if the robots are currently not moving
        private boolean isIdle = true;

    /**
     * Function that generates coordinates for the map walls.
     */
    private void makeWalls() {
        walls.add(new Obstacle(0, 0, WIDTH, WALL_THICKNESS));
        walls.add(new Obstacle(WIDTH - WALL_THICKNESS, 0, WALL_THICKNESS, HEIGHT));
        walls.add(new Obstacle(0, HEIGHT - WALL_THICKNESS, WIDTH - WALL_THICKNESS, WALL_THICKNESS));
        walls.add(new Obstacle(0, WALL_THICKNESS, WALL_THICKNESS, HEIGHT - WALL_THICKNESS));
    }

    /**
     * Attempts to add a single obstacle to the coordinate system.
     * 
     * @param obstacle the obstacle to be placed
     * @param type the type of the obstacle
     * @return true if the obstacle is placed successfully, false otherwise
     */
    private boolean placeObstacle(Obstacle obstacle, Obstacles type) {
        //if the obstacle is too close to other obstacles, abort placement and confirm failure
        if (obstacle.isCloseTo(hills) || obstacle.isCloseTo(rocks)) {
            return false;
        }

        //choose the list to which the obstacle belongs based on the type
        switch (type) {
            case CLIFF -> cliffs.add(obstacle);
            case ROCK -> rocks.add(obstacle);
            case HILL -> hills.add(obstacle);
        }

        //confirm placement success
        return true;
    }

    /**
     * Initializes an obstacle with random values for its location and dimensions
     * 
     * @param type the type of the obstacle to intialize
     * @param x the x coordinate of the obstacle location
     * @param y the y coordinate of the obstacle location
     * @return the initialized obstacle
     */
    private Obstacle createObstacle(Obstacles type, int x, int y) {
        Obstacle obstacle = null;

        //randomly generated dimensions
        final int ROCK_SIZE = random.nextInt(10, 30);
        final int HILL_SIZE = random.nextInt(30, 70);
        final int ROW_DEVIATION = random.nextInt(-10, 50);
        final int COLUMN_DEVIATION = random.nextInt(-10, 50);

        //choose obstacle initializer based on type
        switch(type) {
            case CLIFF -> obstacle = new Obstacle(x, y, random.nextInt(40, 70));
            case ROCK -> obstacle = new Obstacle(x, y, ROCK_SIZE, ROCK_SIZE);
            case HILL -> obstacle = new Obstacle(x, y, HILL_SIZE + ROW_DEVIATION, HILL_SIZE + COLUMN_DEVIATION);
        }

        return obstacle;
    }

    /**
     * Generates a random number of obstacles of a given type and 
     * stores the points in the coordinaate system.
     * @param type the type of the obstacle (cliff, rock or hill)
     */
    private void generate(Obstacles type) {
        int max = random.nextInt(2, 5);
        int count = 0;
        int fails = 0;

        while (count <= max) {
            //starting point
            final Point start = new Point(
                random.nextInt(120, WIDTH - 120),
                random.nextInt(100, HEIGHT - 100)
            );

            //initialize the obstacle
            Obstacle obstacle = createObstacle(type, start.x, start.y);

            //attempt obstacle placement
            boolean placed = placeObstacle(obstacle, type);

            //if placement fails more than 50 times, decrease the maximum amount of required obstacles
            if (placed) {
                count++;
            } else if (fails <= 50) {
                fails++;
            } else {
                fails = 0;
                max--;
            }
        }
    }

    /**
     * Fills additional detail to the generated hills.
     * 
     * @param length the old length of the list containing hills
     */
    private void refineHills(int length) {
        for (int i = 0; i < length; i++) {
            final int rng = random.nextInt(100);
            Obstacle obstacle = hills.get(i);

            if (rng % 13 == 0) {
                int rowDev = random.nextInt(obstacle.width - 5);
                int colDev = random.nextInt(obstacle.height - 5);

                hills.add(new Obstacle(
                    obstacle.x + rowDev, 
                    obstacle.y + colDev,
                    random.nextInt(40, 50), 
                    random.nextInt(40, 50)
                ));
            }
        }
    }

    /**
     * Adds all types of obstacles of random dimensions to the map,
     * at random locations.
     */
    private void randomizeMap() {
        makeWalls();
        generate(Obstacles.HILL);

        //random number for the amount of applied refinements
        int refineCount = random.nextInt(15, 25);

        for (int i = 0; i < refineCount; i++) {
            refineHills(hills.size());
        }
        generate(Obstacles.ROCK);
        generate(Obstacles.CLIFF);
    }

    /**
     * Adds all the necessary simulation elements toghether,
     * including the actual map, the drawn map, the robots, 
     * the obstacles and all the essential GUI elements.
     */
    private void buildSimulation() {
        JPanel content = new JPanel();

        //initialize the camera
        this.camera = new JFrame("Simulation");
        
        randomizeMap();

        //add the drawn map object
        this.trailPanel = new TrailPanel();

        List<Robot> robots = new ArrayList<>();

        //initialize the robots with opposite orientations
        robots.add(new Robot(spawn.x, spawn.y, 0));
        robots.add(new Robot(spawn.x + Robot.WIDTH + 1, spawn.y + Robot.HEIGHT + 1, Math.PI));
        //robots.add(new Robot(spawn.x - Robot.WIDTH - 1, spawn.y - Robot.HEIGHT - 1, Math.PI / 2));

        //initialize the map 
        this.map = new Map(rocks, hills, walls, cliffs, robots, trailPanel);
        this.map.setBackground(GND_COLOR);

        //Add the actual map and the drawn map to the content pane
        content.setLayout(new BoxLayout(content, BoxLayout.LINE_AXIS));
        content.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        content.add(map);
        content.add(trailPanel);
        
        //initialize the camera
        camera.setContentPane(content);
        camera.pack();
        camera.setSize(WIDTH + HEIGHT + 370, HEIGHT + 60);
        camera.setVisible(true);
        camera.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    }

    /**
     * Clears all simulation data.
     */
    private void clearAll() {
        hills.clear();
        rocks.clear();
        walls.clear();
        cliffs.clear();
        map.clearAll();
    }

    /**
     * Updates the visibility of a collection of obstacles
     * @param obs the collection of obstacles
     */
    private void setObstacleVisibility(List<Obstacle> obs) {
        for (Obstacle o : obs) {
            o.setVisible(madeVisible);
        }
    }

    /**
     * Toggles the visibility of all obstacles
     */
    private void toggleVisibility() {
        madeVisible = !madeVisible;
        setObstacleVisibility(hills);
        setObstacleVisibility(rocks);
        setObstacleVisibility(walls);
        setObstacleVisibility(cliffs);
    }

    public static void main(String[] a) {
        Simulation s = new Simulation();
        s.buildSimulation();

        KeyboardFocusManager.getCurrentKeyboardFocusManager().addKeyEventDispatcher(new KeyEventDispatcher() {
            @Override
            public boolean dispatchKeyEvent(KeyEvent e) {
                synchronized (Simulation.class) {
                    final int id = e.getID();
                    final int keyCode = e.getKeyCode();

                    //Reset the simulation if R is pressed
                    if (id == KeyEvent.KEY_PRESSED && keyCode == KeyEvent.VK_R) {
                        s.camera.dispose();
                        s.clearAll();
                        s.spawn.setLocation(
                            s.random.nextInt(WALL_THICKNESS + 1, WIDTH - WALL_THICKNESS - 1),
                            s.random.nextInt(WALL_THICKNESS + 1, HEIGHT - WALL_THICKNESS - 1)
                        );
                        s.buildSimulation();
                        s.madeVisible = false;
                        s.isIdle = true;

                    //toggle the visibility of all obstacles if V is pressed
                    } else if (id == KeyEvent.KEY_PRESSED && keyCode == KeyEvent.VK_V)  {
                        s.toggleVisibility();
                        s.map.repaint();

                    //toggle the automated movement of the robots if S is pressed
                    } else if (id == KeyEvent.KEY_PRESSED && keyCode == KeyEvent.VK_S) {
                        if (s.isIdle) {
                            s.map.autoMove.start();
                            s.isIdle = false;
                        } else {
                            s.map.autoMove.stop();
                            s.isIdle = true;
                            java.util.Map<Point, Color> obs = s.trailPanel.getObs();
                            java.util.Set<Point> trail = s.trailPanel.getTrail();
                        }
                    } 
                }
                return false;
            }
        });
    }
}
