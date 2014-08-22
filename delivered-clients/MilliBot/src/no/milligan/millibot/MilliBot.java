/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot;

import java.io.IOException;
import java.net.UnknownHostException;
import no.milligan.millibot.singletons.Controller;
import no.milligan.millibot.socket.SocketHandler;

/**
 *
 * @author milligan
 */
public class MilliBot {
    public static void main(String[] args) throws UnknownHostException, IOException {
        SocketHandler h = new SocketHandler();
        
        // Starting the socket listener!
        Thread t = new Thread(h);
        t.start();
        
        // Let's register with the server
        Controller.getInstance().sendRawCommand("name millibot");
        // Set our spawn mode straight away. Spawn a couple harvesters to get us going
        Controller.getInstance().sendRawCommand("{\"mode\": \"harvester\"}");
        //h.sendCommand("name MilliBot");
    }
}
