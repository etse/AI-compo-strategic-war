/*
 * Code by Kristoffer Milligan
 * kristoffer.milligan@marlink.com
 */
package no.milligan.millibot.tools;

import java.awt.Point;
import java.util.ArrayList;
import java.util.LinkedList;
import no.milligan.millibot.singletons.Controller;

public class PathFinder {
    private LinkedList m_openList;
    private LinkedList m_closedList;

    public PathFinder(){
        this.m_openList = new LinkedList();
        this.m_closedList = new LinkedList();
    }

    public LinkedList findPath(Point start, Point goal) throws NullPointerException, ClassNotFoundException {
        this.m_openList.clear();
        this.m_closedList.clear();

        Node goalNode = new Node(goal, null, null);
        Node startNode = new Node(start, null, null);
        Node temp = new Node(start, null, goalNode);

        this.m_openList.add(temp);
        int run_count = 0;
        while(!this.m_openList.isEmpty()){
            if(temp.getLocation().equals(goal) || run_count > 256){ // This can be adjusted for less/more accurate path finding. The higher the number, the longer each calculation will take
                    return this.constructPath(temp);
            }
            temp = this.lookingForBestNode();
            this.m_closedList.addLast(temp);
            this.addNeighbor(temp, startNode, goalNode);
            run_count++;
        }

        return null;
    }
    
    public Point nextHop(Point start, Point goal) throws NullPointerException, ClassNotFoundException {
        this.m_openList.clear();
        this.m_closedList.clear();
        
        Node goalNode = new Node(goal, null, null);
        Node startNode = new Node(start, null, null);
        Node temp = new Node(start, null, goalNode);

        this.m_openList.add(temp);
        while(!this.m_openList.isEmpty()){
            if(temp.getLocation().equals(goal)){
                    LinkedList l = this.constructPath(temp);
                    String values[];
                    if(l.size()>1)
                        values = l.get(1).toString().split("\t")[0].replace("[", "").replace("]","").split(",");
                    else
                        values = l.get(0).toString().split("\t")[0].replace("[", "").replace("]","").split(",");
                    
                    return new Point(Integer.parseInt(values[0]),Integer.parseInt(values[1]));
            }
            temp = this.lookingForBestNode();
            this.m_closedList.addLast(temp);
            this.addNeighbor(temp, startNode, goalNode);
        }

        return null;
    }

    private void addNeighbor(Node parent, Node start, Node goal){
        int x = parent.getLocation().x;
        int y = parent.getLocation().y;

        for(int leftTopY = y - 1; leftTopY < y + 2; leftTopY++){
            for(int leftTopX = x - 1; leftTopX < x + 2; leftTopX++){
                try{
                    if(leftTopY == y && leftTopX == x){
                        //pass itself
                    } else if(leftTopY == y-1 && leftTopX == x-1){
                        //pass top left
                    } else if(leftTopY == y-1 && leftTopX == x+1){
                        //pass top right
                    } else if(leftTopY == y+1 && leftTopX == x+1){
                        //pass bottom right
                    } else if(leftTopY == y+1 && leftTopX == x-1){
                        //pass bottom left
                    }
                    //else if(!map.getTile(leftTopX, leftTopY).isObstacle()){
                    else if(!Controller.getInstance().grid.isBlocked(new Point(leftTopX,leftTopY))){
                        Node node = new Node(new Point(leftTopX, leftTopY), parent, goal);
                        int index = this.openListIndexOf(node);
                        if(this.closedListContains(node)){
                            //pass
                        }
                        else if(index != -1){
                            Node old = (Node) this.m_openList.get(index);
                            if(old.getParent().getCostFromStart() > node.getParent().getCostFromStart()){
                                this.m_openList.set(index, node);
                            }
                        }
                        else{
                            this.m_openList.add(node);
                        }
                    }
                }
                catch(ArrayIndexOutOfBoundsException e){
                        //just pass
                }
            }
        }
    }

    private int openListIndexOf(Node node){
        for(int index = 0; index < this.m_openList.size(); index++){
            Node anotherNode = (Node) this.m_openList.get(index);
            if(anotherNode.equals(node)){
                    return index;
            }
        }
        return -1;
    }

    private boolean openListContains(Node node){
        for(int index = 0; index < this.m_openList.size(); index++){
            Node anotherNode = (Node) this.m_openList.get(index);
            if(anotherNode.equals(node)){
                return true;
            }
        }
        return false;
    }

    private boolean closedListContains(Node node){
        for(int index = 0; index < this.m_closedList.size(); index++){
            Node anotherNode = (Node) this.m_closedList.get(index);
            if(anotherNode.equals(node)){
                return true;
            }
        }
        return false;
    }

    private Node lookingForBestNode(){
        if(this.m_openList.isEmpty()){
            return null;
        }
        int lowestCostIndex = 0;
        float cost = ((Node)this.m_openList.get(0)).getTotalCost();
        for(int index = 1; index < this.m_openList.size(); index++){
            Node node = (Node) this.m_openList.get(index);
            if(node.getTotalCost() < cost){
                    cost = node.getTotalCost();
                    lowestCostIndex = index;
            }	
        }
        Node node = (Node) this.m_openList.remove(lowestCostIndex);
        return node;
    }

    private LinkedList constructPath(Node goal){
        LinkedList path = new LinkedList();
        while(goal != null){
            path.addFirst(goal);
            goal = goal.getParent();
        }
        return path;
    }

    public class Node{
        private Node m_parent;
        private Point m_location;
        private float m_costFromStart;
        private float m_costToGoal;

        public Node(Point location, Node parent, Node goal){
            this.m_parent = parent;
            this.m_location = location;
            float x;
            float y;
            if(parent != null){
                this.m_costFromStart = this.calculateCostFromStart(parent);
            }
            if(goal != null){
                x = location.x - goal.getLocation().x;
                y = location.y - goal.getLocation().y;
                this.m_costToGoal = (float) Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2));
            }
        }

        private float calculateCostFromStart(Node parent){
            float x = this.getLocation().x - parent.getLocation().x;
            float y = this.getLocation().y - parent.getLocation().y;
            float sum = (float) Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2));
            sum += this.getParent().getCostFromStart();
            return sum;
        }

        public boolean equals(Node node){
            return this.m_location.equals(node.getLocation());
        }

        public Node getParent(){
            return this.m_parent;
        }

        public Point getLocation(){
            return this.m_location;
        }

        public float getTotalCost(){
            return this.m_costFromStart + this.m_costToGoal;
        }

        public float getCostFromStart(){
            return this.m_costFromStart;
        }

        public float getCostToGoal(){
            return this.m_costToGoal;
        }

        public String getParentMessage(){
            if(this.m_parent == null){
                return null;
            }
            return "[" + this.m_parent.getLocation().x + "," + this.m_parent.getLocation().y + "]";
        }

        public String getLocationMessage(){
            return "[" + this.m_location.x + "," + this.m_location.y + "]";
        }

        public String toString(){
            return "[" + this.m_location.x + "," + this.m_location.y + "]" + "\tparent: " + this.m_parent;
        }
    }
}