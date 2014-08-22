/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */

package no.milligan.millibot.objects;

import no.milligan.millibot.singletons.Controller.EntityType;

/**
 *
 * @author milligan
 */
public class Wall extends Entity {
    public Wall() {
        this.type = EntityType.WALL;
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
