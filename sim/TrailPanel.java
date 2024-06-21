import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.Map;

import javax.swing.JPanel;

/**
 * Subclass of JPanel designed to provide graphics for the territory
 * mapped by the robots during their exploration around Venus.
 */
public class TrailPanel extends JPanel {
    private Set<Point> trailPoints; //set containing visited points
    private Map<Point, Color> detectedObstacles; // Use a map to store detected points and their colors

    //returns the trail points
    public Set<Point> getTrail() {
        return this.trailPoints;
    }

    //default constructor
    public TrailPanel() {
        this.trailPoints = new HashSet<>();
        this.detectedObstacles = new HashMap<>();
        setBackground(Color.darkGray);
    }

    /**
     * adds a point to the set of travelled points
     * @param p the point to be added
     */
    public void addPoint(Point p) {
        trailPoints.add(p);
        repaint();
    }

    /**
     * adds a point to the set of discovered obstacle points
     * @param p the point to be added
     * @param color the desired color
     */
    public void addObstacle(Point p, Color color) {
        detectedObstacles.put(p, color);
        repaint();
    }

    /**
     * checks if a specified point is already drawn
     * @param p the given point
     * @return true if the point is drawn, false otherwise
     */
    public boolean has(Point p) {
        return this.trailPoints.contains(p) || this.detectedObstacles.containsKey(p);
    }

    /**
     * Clears all data.
     */
    public void clearAll() {
        this.detectedObstacles.clear();
        this.trailPoints.clear();
    }

    public Map<Point, Color> getObs() {
        return this.detectedObstacles;
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        g.setColor(Color.orange);

        for (Point p : trailPoints) {
            g.fillOval(p.x, p.y, 5, 5);
        }

        for (Map.Entry<Point, Color> entry : detectedObstacles.entrySet()) {
            g.setColor(entry.getValue());
            Point p = entry.getKey();
            g.fillOval(p.x, p.y, 5, 5);
        }
    }
}
