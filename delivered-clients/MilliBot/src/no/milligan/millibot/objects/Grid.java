/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot.objects;

import java.awt.Point;
import java.util.ArrayList;
import no.milligan.millibot.singletons.Controller;
import no.milligan.millibot.singletons.Controller.EntityType;

/**
 *
 * @author milligan
 */
public class Grid {
    private final int height, width;
    private final ArrayList<Wall> grid = new ArrayList<Wall>();
    public Grid(int height, int width) {
        this.height = height;
        this.width = width;
    };
    public void addGrid(Wall entity) {
        boolean add = true;
        for(Wall g : grid) {
            if(g.position == entity.position) {
                add = false;
                break;
            }
        }
        if(add) {
            // This wall doesn't exist. Let's add it!
            grid.add(entity);
        }
    }
    public boolean isBlocked(Point position) {
        for(Wall g : grid) {
            if(g.position.equals(position)) {
                return true;
            }
        }
        for(Unit u : Controller.getInstance().units) {
            if(u.position.equals(position) && (u.type!=EntityType.SPAWNER && u.owner_id!=Controller.getInstance().my_id)) {
                return true; // position is blocked by a unit and it's not an enemy spawner!
            }
        }
        return false;
    }
    public int getHeight() {
        return this.height;
    }
    public int getWidth() {
        return this.width;
    }
}
