/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */

package no.milligan.millibot.objects;

import java.awt.Point;
import java.util.LinkedList;
import no.milligan.millibot.singletons.Controller;
import no.milligan.millibot.singletons.Controller.Direction;
import no.milligan.millibot.singletons.Controller.EntityType;
import no.milligan.millibot.singletons.Controller.WorkStatus;
import no.milligan.millibot.tools.PathFinder;
import no.milligan.millibot.tools.PathFinder.Node;

/**
 *
 * @author milligan
 */
public class Unit extends Entity {
    public Point target_position = this.position;
    public LinkedList moves = new LinkedList();

    /**
     * Returns the manhattan cost 
     * @param target
     * @return 
     */
    public int travel_cost(Entity target) {
        int x = getX();
        int y = getY();
        int cost = 0;
        while(x != target.getX() && y != target.getY()) {
            if(x < target.getX()) {
                x++;
                cost += 10;
            } else if(x > target.getX()) {
                x--;
                cost += 10;
            }
            if(y < target.getY()) {
                y++;
                cost += 10;
            } else if(y > target.getX()) {
                y--;
                cost += 10;
            }
        }
        return cost;
    }
    public void setTarget(Point target) throws NullPointerException, ClassNotFoundException {
        if(type==EntityType.SPAWNER)
            return; // Nah - spawners can't move
        this.target_position = target;
        //System.out.println("["+position+"] Setting my target to ["+target_position+"]");
        if(moves.isEmpty()) {
           // Calculate a path
            //System.out.println("\tCalculating new path ["+target_position+"] because the old one was empty");
            PathFinder p = new PathFinder();
            moves = p.findPath(position, target_position);
            // First is always ourselves. Let's remove it!
            moves.removeFirst();
            //System.out.println("\tMoves set. There are " + moves.size() + " hops");
        } else {
            // There is a premade path here. Does it need recalculation?
            Node node = (Node)moves.getLast();
            if(distance(node.getLocation(),target_position) > distance(position, target_position)) {
            //if(node.getLocation().distance(target_position) > this.position.distance(target_position)) {
                //System.out.println("\tCalculating new path ["+target_position+"] because the old one was wrong");
                // Yep, path is wrong. Let's make a new one!
                PathFinder p = new PathFinder();
                moves = p.findPath(position, target_position);
                // First is always ourselves. Let's remove it!
                moves.removeFirst();
                //System.out.println("\tMoves set. There are " + moves.size() + " hops");
            }
        }
    }
    public Move move() throws NullPointerException, ClassNotFoundException {
        if(type == EntityType.SPAWNER)
            return null; // No need to move a spawner!!
        //System.out.println("["+position+"] Moving");
        if(moves.isEmpty() && !this.target_position.equals(this.position)){ // We don't have any more moves, but we haven't arrived either :(
            PathFinder p = new PathFinder();
            moves = p.findPath(position, target_position);
            if(moves.size()>1)
                moves.removeFirst(); // Removing ourselves!
        }
        if(moves.isEmpty()) {
            // wtf - this shouldn't be possible. No moves available. This fellar is blocked in!
            return null;
        }
        Point next_hop = ((Node)moves.removeFirst()).getLocation();
        if(Controller.getInstance().grid.isBlocked(next_hop)) {
            //System.out.println("\tRecalculating moves because of blocked next hop");
            moves.clear(); // Darn! Need to recalc the next hop here!
            PathFinder p = new PathFinder();
            moves = p.findPath(position, target_position);
            if(moves.size()>1)
                moves.removeFirst(); // Removing ourselves!
            next_hop = ((Node)moves.removeFirst()).getLocation();
        }
        Move move = new Move();
        move.origin = new Point(position.x, position.y);
        if(next_hop.x < position.x) {
            // We need to go west
            move.direction = Direction.WEST;
            position.x--;
        } else if(next_hop.x > position.x) {
            // We need to go east
            move.direction = Direction.EAST;
            position.x++;
        } else if(next_hop.y > position.y) {
            // We need to go south
            move.direction = Direction.SOUTH;
            position.y++;
        } else if(next_hop.y < position.y) {
            // We need to go north
            move.direction = Direction.NORTH;
            position.y--;
        }
        
        return move;
    }

    public int distance(Point target) {
        return ((target.x-getX())*(target.x-getX())) + ((target.y-getY())*(target.y-getY()));
    }
    
    public int distance(Point start, Point target) {
        return ((target.x-start.x)*(target.x-start.x)) + ((target.y-start.y)*(target.y-start.y));
    }
    
    @Override
    public int getX() {
        return (int)this.position.getX();
    }

    @Override
    public int getY() {
        return (int)this.position.getY();
    }
}
