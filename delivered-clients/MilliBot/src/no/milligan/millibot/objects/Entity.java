/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot.objects;

import java.awt.Point;
import no.milligan.millibot.singletons.Controller.EntityType;
import no.milligan.millibot.singletons.Controller.WorkStatus;
/**
 *
 * @author milligan
 */
abstract class Entity {
    public boolean destroyed = false, updated = true;
    public int owner_id = 0;
    public Point position = new Point();
    public EntityType type = null;
    
    abstract public int getX();
    abstract public int getY();
}
