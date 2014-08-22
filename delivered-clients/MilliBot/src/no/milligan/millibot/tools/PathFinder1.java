/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot.tools;

import java.awt.Point;
import java.util.ArrayList;
import no.milligan.millibot.singletons.Controller;
/**
 *
 * @author milligan
 */
public class PathFinder1 {
    private int cost_increase = 10;
    private ArrayList<Step> open_list = new ArrayList<Step>();
    private ArrayList<Step> closed_list = new ArrayList<Step>();
    public void next_step(Point start, Point end) {
        System.out.println("Looking for path to " + end.toString() + " from " + start.toString());
        Step step_one = new Step();
        step_one.position = start;
        step_one.parent = null;
        step_one.cost_from_start = 0;
        step_one.manhattan_cost = travel_cost(start, end);
        
        //open_list.add(step_one);
        //open_list.remove(step_one);
        closed_list.add(step_one);
        
        // Now we should loop!
        while(true) {
            
            System.out.println("Checking from parent " + start);
            
            Point west = new Point(start.x-1, start.y);
            Point east = new Point(start.x+1, start.y);
            Point north = new Point(start.x, start.y-1);
            Point south = new Point(start.x, start.y+1);
            
            if(!Controller.getInstance().grid.isBlocked(west)) {
                Step s = new Step();
                s.manhattan_cost = travel_cost(west, end);
                s.parent = step_one;
                s.position = west;
                s.cost_from_start = s.parent.cost_from_start + this.cost_increase;
                
                if(!open_list.contains(s) && !closed_list.contains(s))
                    open_list.add(s);
            }
            if(!Controller.getInstance().grid.isBlocked(east)) {
                Step s = new Step();
                s.manhattan_cost = travel_cost(east, end);
                s.parent = step_one;
                s.position = east;
                s.cost_from_start = s.parent.cost_from_start + this.cost_increase;
                
                if(!open_list.contains(s) && !closed_list.contains(s))
                    open_list.add(s);
            }
            if(!Controller.getInstance().grid.isBlocked(north)) {
                Step s = new Step();
                s.manhattan_cost = travel_cost(north, end);
                s.parent = step_one;
                s.position = north;
                s.cost_from_start = s.parent.cost_from_start + this.cost_increase;
                
                if(!open_list.contains(s) && !closed_list.contains(s))
                    open_list.add(s);
            }
            if(!Controller.getInstance().grid.isBlocked(south)) {
                Step s = new Step();
                s.manhattan_cost = travel_cost(south, end);
                s.parent = step_one;
                s.position = south;
                s.cost_from_start = s.parent.cost_from_start + this.cost_increase;
                
                if(!open_list.contains(s) && !closed_list.contains(s))
                    open_list.add(s);
            }
            
            Step lowest_cost = null;
            for(Step s : open_list) {
                if(lowest_cost == null) {
                    lowest_cost = s;
                    continue;
                }
                if(s.cost_sum() < lowest_cost.cost_sum()) {
                    lowest_cost = s;
                }
            }
            // We now have the lowest cost square.
            open_list.remove(lowest_cost);
            closed_list.add(lowest_cost);
            
            if(lowest_cost.position.equals(end)) {
                System.out.println("We hit our target!");
                break;
            }
        }
        for(Step s : closed_list) {
            System.out.println("Cost:"+s.cost_from_start+" Manhattan_cost:"+s.manhattan_cost+" Parent:"+s.parent+" Self:"+s.position+" Sum:"+s.cost_sum());
        }
        System.exit(0);
    }
    private class Step {
        public Point position;
        public Step parent;
        public int cost_from_start, manhattan_cost;
        public Step(){};
        public int cost_sum() {
            return cost_from_start + manhattan_cost;
        }
    }
    private int travel_cost(Point start, Point end) {
        int x = (int)start.getX();
        int y = (int)start.getY();
        int cost = 0;
        while(x != end.getX() && y != end.getY()) {
            if(x < end.getX()) {
                x++;
                cost += 10;
            } else if(x > end.getX()) {
                x--;
                cost += 10;
            }
            if(y < end.getY()) {
                y++;
                cost += 10;
            } else if(y > end.getX()) {
                y--;
                cost += 10;
            }
        }
        return cost;
    }
}
