/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot.singletons;

/**
 *
 * @author milligan
 */
public class DataHandler {
    private static DataHandler instance = null;
    protected DataHandler() {    
    }
    public static DataHandler getInstance() {
        if(instance == null)
            instance = new DataHandler();
        
        return instance;
    }
    public void handleData(String key, String value) {
    }
}