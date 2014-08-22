/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */

package no.milligan.millibot.objects;

import java.awt.Point;
import no.milligan.millibot.singletons.Controller.Direction;

/**
 *
 * @author milligan
 */
public class Move {
    public Point origin;
    public Direction direction;
    public Move() {
        origin = new Point();
    }
}
