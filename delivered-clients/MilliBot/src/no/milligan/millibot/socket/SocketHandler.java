/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot.socket;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import java.awt.Point;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import no.milligan.millibot.objects.Food;
import no.milligan.millibot.objects.Grid;
import no.milligan.millibot.objects.Unit;
import no.milligan.millibot.objects.Wall;
import no.milligan.millibot.singletons.Controller;
import no.milligan.millibot.singletons.Controller.EntityType;

/**
 *
 * @author milligan
 */
public class SocketHandler implements Runnable {
    private Socket mysoc;
    private PrintWriter out;
    private BufferedReader in; 
    
    public SocketHandler() throws UnknownHostException, IOException {
        mysoc = new Socket("127.0.0.1",5050);
        out = new PrintWriter(mysoc.getOutputStream(), true);
        in = new BufferedReader(new InputStreamReader(mysoc.getInputStream()));
        
        Controller.getInstance().setSocketPipe(new PrintWriter(mysoc.getOutputStream(), true));
    }

    @Override
    public void run() {
        // Keep running forever!
        System.out.println("Connecting to server");
        String line;
        while(true) {
            try {
                //System.out.println("["+System.currentTimeMillis()+"] Waiting for line");
                line = in.readLine();
                if(!line.trim().equals("")) {
                    if(line.trim().equals("{\"status\": \"OK\"}")){
                        continue;
                    }
                    long start = System.currentTimeMillis();
                    System.out.println("["+start+"] Started line parsing");
                    JsonElement element = new JsonParser().parse(line);
                    JsonObject obj = element.getAsJsonObject();
                    
                    Controller.getInstance().resetLists();
                    
                    for (Map.Entry pairs : obj.entrySet()) {
                        String key = pairs.getKey().toString();
                        switch(key) {
                            case("map_size"):
                                if(Controller.getInstance().grid==null) {
                                    String[] value = pairs.getValue().toString().replace("[", "").replace("]", "").split(",");
                                    Controller.getInstance().grid = new Grid(Integer.parseInt(value[0]),Integer.parseInt(value[1]));
                                }
                                break;
                            case("player_id"):
                                Controller.getInstance().my_id = Integer.parseInt(pairs.getValue().toString());
                                break;
                            case("map"):
                                // We know for sure that this information has a position
                                
                                JsonArray arr = (JsonArray) pairs.getValue();
                                for(int i = 0; i < arr.size(); i++) {
                                    JsonObject e = arr.get(i).getAsJsonObject();
                                    //System.out.println(e.toString());

                                    String[] position_value = e.get("position").toString().replace("[", "").replace("]", "").split(","); // Should contain a X Y representation of the position
                                    if(e.has("is_wall")) {
                                        // this is wall information
                                        Wall u = new Wall();
                                        u.position = (new Point(Integer.parseInt(position_value[0]),Integer.parseInt(position_value[1])));
                                        u.type = EntityType.WALL;
                                        Controller.getInstance().grid.addGrid(u);
                                    } else if(e.has("has_food")) {
                                        // this is food information
                                        Food u = new Food();
                                        u.position = (new Point(Integer.parseInt(position_value[0]),Integer.parseInt(position_value[1])));
                                        u.type = EntityType.FOOD;
                                        Controller.getInstance().food.add(u);
                                    } else {
                                        // This is unit or spawner information
                                        Unit u = new Unit();
                                        u.position = (new Point(Integer.parseInt(position_value[0]),Integer.parseInt(position_value[1])));
                                        u.target_position = u.position;
                                        if(e.has("spawner")) {
                                            JsonObject spawner = e.getAsJsonObject("spawner");
                                            u.type = EntityType.SPAWNER;
                                            u.destroyed = spawner.get("destroyed").getAsBoolean();
                                            u.owner_id = spawner.get("owner").getAsInt();
                                        } 
                                        if(e.has("unit")) {
                                            //System.out.println(e.toString());
                                            JsonObject unit = e.getAsJsonObject("unit");
                                            switch(unit.get("type").getAsString()) {
                                                case("unit"):
                                                    u.type = EntityType.UNIT;
                                                    break;
                                                case("soldier"):
                                                    u.type = EntityType.SOLDIER;
                                                    break;
                                                case("harvester"):
                                                    u.type = EntityType.HARVESTER;
                                                    break; 
                                            }
                                            u.owner_id = unit.get("owner").getAsInt();
                                        }
                                        Controller.getInstance().addUnit(u, line);
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    // We're done reading a line. Let's hand information over to the controller
                    if(Controller.getInstance().grid != null) {
                        Controller.getInstance().doCalculations();
                    }
                    long end = System.currentTimeMillis();
                    System.out.println("["+end+"] Ended line parsing");
                    
                    System.out.println("["+(end-start)+"] ms\n");
                }
            } catch (IOException ex) {
                Logger.getLogger(SocketHandler.class.getName()).log(Level.SEVERE, null, ex);
            } catch (NullPointerException ex) {
                Logger.getLogger(SocketHandler.class.getName()).log(Level.SEVERE, null, ex);
            } catch (ClassNotFoundException ex) {
                Logger.getLogger(SocketHandler.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
}
