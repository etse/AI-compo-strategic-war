/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot.singletons;

import com.google.gson.Gson;
import com.google.gson.JsonElement;
import java.awt.Point;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Random;
import no.milligan.millibot.objects.Food;
import no.milligan.millibot.objects.Grid;
import no.milligan.millibot.objects.Move;
import no.milligan.millibot.objects.Unit;
import no.milligan.millibot.tools.PathFinder;
import no.milligan.millibot.tools.PathFinder.Node;

/**
 *
 * @author milligan
 */
public class Controller {
    private static Controller instance = null;
    private PrintWriter socket_pipe = null;
    private Gson gson = new Gson();
        
    private ArrayList<Move> used_next_hops;
    
    
    private String spawn_mode = "harvester";
    
    public int my_id = -1;
    
    public ArrayList<String> commands = new ArrayList<String>();
    
    public Grid grid = null;
    
    public ArrayList<Unit> units = new ArrayList<Unit>();
    public ArrayList<Food> food = new ArrayList<Food>();
    
    protected Controller() {      
        this.used_next_hops = new ArrayList<Move>();
    }
    public void setSocketPipe(PrintWriter writer) {
        this.socket_pipe = writer;
    }
    public static Controller getInstance() {
        if(instance==null)
            instance = new Controller();
        return instance;
    }
    
    public void sendRawCommand(String command) {
        //System.out.println("Sending RAW command " + command);
        socket_pipe.write(command);
        socket_pipe.write("\n");
        socket_pipe.flush();
    }
    
    public void sendJson(JsonElement element) {
        //System.out.println("Going to send JSON element");
        ///System.out.println(gson.toJson(element));
        //System.out.println();
        sendRawCommand(gson.toJson(element)+"\n");
    }
    
    public void doCalculations() throws NullPointerException, ClassNotFoundException {
        
        cleanLists(); // This should remove any unit that hasn't received an update, so we don't know anything about it anymore.
        
        //System.out.println("We have " + units.size() + " units");
        //System.out.println("We have " + food.size() + " food in sight");
        // Let's check how many harvesters we have
        if(count(EntityType.HARVESTER)<4) {
            this.spawn_mode = "harvester";
        } else if(count(EntityType.UNIT) < 5) {
            this.spawn_mode = "standard";
        } else {
            this.spawn_mode = "soldier";
        }
        
        String command = "{\"mode\": \""+spawn_mode+"\", \"moves\": [";
        
        for(Food f : food) {
            // Looping each food that we know about. Make the closest unit gather it.
            Unit gatherer = null;
            if(count(EntityType.HARVESTER) > 0) {
                // We have a harvester - let's make it gather the food!
                for(Unit u : getEntities(EntityType.HARVESTER)) {
                    if(gatherer == null) {
                        gatherer = u;
                        continue;
                    }
                    if(gatherer.travel_cost(f) > u.travel_cost(f)){
                        gatherer = u;
                    }
                }
            } else {
                for(Unit u : units) {
                    if(gatherer == null) {
                        gatherer = u;
                        continue;
                    }
                    if(gatherer.travel_cost(f) > u.travel_cost(f)){
                        gatherer = u;
                    }
                }
            }
            // We know which unit is the closest. Let's move him!
            if(gatherer!=null) {
                //System.out.println("Setting target position for unit " + gatherer.position + " to " + f.position);
                if(!gatherer.target_position.equals(f.position)) {
                    // Oh, new gatherer! Clear out his movement list so he can recalc moves!
                    gatherer.moves.clear();
                }
                //System.out.println("[SYSTEM] Setting target for " + gatherer.position);
                gatherer.setTarget(f.position);
            }
        }
        
        used_next_hops.clear();
        
        for(Unit u : units) {
            if(u.owner_id!=my_id) {
                // Enemy unit - let's killlllll
                if(u.type==EntityType.HARVESTER) {
                    // We can use any soldier or unit
                    Unit attacker = null;
                    for(Unit att : getEntities(EntityType.UNIT,EntityType.SOLDIER)) {
                        if(attacker==null) {
                            attacker = att;
                            continue;
                        }
                        if(att.distance(u.position)<attacker.distance(u.position))
                            attacker = att;
                    }
                    if(attacker!=null){
                        System.out.println("\tAttacking harvester at ["+u.position+"]");
                        attacker.setTarget(u.position); // Moving the closest unit to the spawner!
                    }
                } else if(u.type==EntityType.UNIT) {
                    // We can use unit pairs or a single soldier
                    Unit attacker = null;
                    for(Unit att : getEntities(EntityType.UNIT,EntityType.SOLDIER)) {
                        if(attacker==null) {
                            attacker = att;
                            continue;
                        }
                        if(att.distance(u.position)<attacker.distance(u.position))
                            attacker = att;
                    }
                    if(attacker!=null){
                        System.out.println("\tAttacking unit at ["+u.position+"]");
                        attacker.setTarget(u.position); // Moving the closest unit to the spawner!
                    }
                } else if(u.type==EntityType.SOLDIER) {
                    // We need to use soldier pairs
                    Unit attacker = null;
                    for(Unit att : getEntities(EntityType.SOLDIER)) {
                        if(attacker==null) {
                            attacker = att;
                            continue;
                        }
                        if(att.distance(u.position)<attacker.distance(u.position))
                            attacker = att;
                    }
                    if(attacker!=null){
                        System.out.println("\tAttacking soldier at ["+u.position+"]");
                        attacker.setTarget(u.position); // Moving the closest unit to the spawner!
                    }
                } else {
                    // This is a spawner. Let's just send someone to sit on it!
                    if(!u.destroyed) {
                        Unit attacker = null;
                        for(Unit att : getEntities(EntityType.UNIT,EntityType.SOLDIER)) {
                            if(attacker==null) {
                                attacker = att;
                                continue;
                            }
                            if(att.distance(u.position)<attacker.distance(u.position))
                                attacker = att;
                        }
                        if(attacker!=null){
                            System.out.println("\tAttacking spawner at ["+u.position+"]");
                            attacker.setTarget(u.position); // Moving the closest unit to the spawner!
                        }
                    }
                }
                continue;
            }
            System.out.println("Unit ["+u.position+"]");
            System.out.println("\t["+u.target_position+"] target position");
            System.out.println("\t["+u.moves.size()+"] moves in list.");
            if(u.moves.size()==1) {
                System.out.println("\t["+((Node)u.moves.getFirst()).getLocation()+"] is the last one.");
            }
            System.out.println("\t["+u.type+"]");
            System.out.println("\t["+u.owner_id+"]");
            if(u.position.equals(u.target_position) && u.moves.isEmpty()) { // We have arrived at position and our moves list is empty!
                // Are we sitting on a spawner?
                for(Unit enemy : units) {
                    if(enemy.owner_id==my_id || enemy.type!=EntityType.SPAWNER)
                        continue;
                    // OK, we've found a spawner. Is this the same position as this unit?
                    if(enemy.position.equals(u.position) && !enemy.destroyed) {
                        // Unit should just sit - spawner isn't destroyed yet
                        u.setTarget(enemy.position);
                        System.out.println("\tSticking to point to kill spawner.");
                        break;
                    }
                }
                // this unit is just chillaxing. Let's explore instead. Let's just set a position somewhere
                if(u.moves.isEmpty()) {
                    u.setTarget(new Point(randInt(1,this.grid.getWidth()), randInt(1, grid.getHeight())));
                    System.out.println("\tSetting random movement point.");
                }
                //System.out.println("Giving ["+u.position+"] a random target ["+u.target_position+"]");
            }
            Move m = u.move();
            if(m==null) {
                // Didn't receive a new hop. Okidoki. Might be a spawner, or a camper
            } else {
                if(!used_next_hops.contains(m)) {
                    used_next_hops.add(m); // Added!
                } else {
                    System.out.println("[SYSTEM] DID NOT ADD ["+m.origin+","+m.direction+"] - would cause a crash!");
                }
            }
            System.out.println("\tPost move ["+u.position+"]");
        }
        // OK, we have a list over all moves now!
        for(Move move : used_next_hops) {
            command += "["+move.origin.x+","+move.origin.y+",\""+move.direction.toString().toLowerCase()+"\"],";
        }
        
        command += "]}";
        command = command.replace(",]}", "]}");
        System.out.println("Sending " + command);
        this.sendRawCommand(command);
    }
    
    public void addUnit(Unit unit, String line) {
        //System.out.println("Checking unit " + unit.position);
        for(Unit u : units) {
            if(u.position.equals(unit.position) && u.owner_id == unit.owner_id) {
                if(u.type!=unit.type) {
                    //System.out.println("Received new unit type on location. Probably a unit spawn? " + u.position + "("+u.type.toString()+")");
                    u = unit;
                    u.updated = true;
                } else {
                    //System.out.println("Updating " + u.position + "("+u.type.toString()+")");
                    u.updated = true;
                }
                //System.out.println("\tExists " + u.position);
                return;
            }
        }
        //System.out.println("\t Doesnt exist ");
        //System.out.println("\t "+line);
        //System.out.println("Adding " + unit.position + "("+unit.type.toString()+")");
        units.add(unit);
    }
    public void addFood(Food foods) {
        for(Food u : food) {
            if(u.position.equals(foods.position)) {
                u.updated = true;
                //System.out.println("Received info food " + u.position);
                return;
            }
        }
        // OK - we don't have a unit in this position. Let's add it!
        food.add(foods);
    }
    
    public void resetLists() {
        for(Unit u : units) {
            u.updated = false;
        }
        for(Food u : food) {
            u.updated = false;
        }
    }
    public void cleanLists() {
        for (Iterator<Unit> it = units.iterator(); it.hasNext(); ) {
            Unit u = it.next();
            if (!u.updated) {
                it.remove();
            }
        }
        for (Iterator<Food> it = food.iterator(); it.hasNext(); ) {
            Food u = it.next();
            if (!u.updated) {
                it.remove();
            }
        }
    }
    
    public int count(EntityType type) {
        int count = 0;
        for(Unit u : units) {
            if(u.type==type && u.owner_id == my_id)
                count++;
        }
        return count;
    }
    public ArrayList<Unit> getEntities(EntityType type) {
        ArrayList<Unit> h = new ArrayList<Unit>();
        for(Unit u : units) {
            if(u.type==type && u.owner_id == my_id)
                h.add(u);
        }
        return h;
    }
    public ArrayList<Unit> getEntities(EntityType type1, EntityType type2) {
        ArrayList<Unit> h = new ArrayList<Unit>();
        for(Unit u : units) {
            if((u.type==type1 || u.type==type2) && u.owner_id == my_id)
                h.add(u);
        }
        return h;
    }
    
    public static int randInt(int min, int max) {
        // NOTE: Usually this should be a field rather than a method
        // variable so that it is not re-seeded every call.
        Random rand = new Random();
        // nextInt is normally exclusive of the top value,
        // so add 1 to make it inclusive
        int randomNum = rand.nextInt((max - min) + 1) + min;
        return randomNum;
    }
    
    public static enum SpawnMode {
        HARVESTER,
        SOLDIER,
        UNIT;
    }
    public static enum Direction {
        NORTH,
        SOUTH,
        EAST,
        WEST;
    }
    public static enum EntityType {
        SPAWNER,
        UNIT,
        SOLDIER,
        HARVESTER,
        WALL,
        FOOD;
    }
    public static enum WorkStatus {
        FARMING,
        SCOUTING,
        STAND_STILL,
        WAR;
    }
}
