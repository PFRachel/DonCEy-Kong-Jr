package domain;

public class DKJr {
    private float x = 100;
    private float y = 400;
    //private float velocityX = 0;
    //private float velocityY = 0;
    //private float gravity = -300f;
    private float speed = 200f;

    private boolean onLiana = false;
    private int currentLiana = -1;
    private boolean isAlive = true;

    private float minX = 0, maxX = 830;
    private float minY = 0, maxY = 700;

    public float getX() { return x; }
    public float getY() { return y; }
    //public boolean isOnLiana() { return onLiana; }
    //public int getCurrentLiana() { return currentLiana; }
    //public boolean isAlive() { return isAlive; }
    
    // Posicion inicial
    public void setPosition(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public void applyInput(int up, int down, int left, int right, int jump, float dt) {

        float move = speed * dt;

        // Movimiento básico
        if (right == 1) x += move;
        if (left == 1) x -= move;
        if (up == 1) y -= move;
        if (down == 1) y += move;

        // Límites
        if (x < minX) x = minX;
        if (x > maxX) x = maxX;
        if (y < minY) y = minY;
        if (y > maxY) y = maxY;
    }

    private void verificarLimites() {
        // Límites horizontales
        if (x < minX) x = minX;
        if (x > maxX) x = maxX;
        
        // Colisión con el suelo
        if (y > maxY) {
            y = maxY;
            //velocityY = 0;
        }
        
        // Techo
        if (y < minY) {
            y = minY;
            //velocityY = 0;
        }
    }

    private void clamp() {
        if (x < minX) x = minX;
        if (x > maxX) x = maxX;

        if (y < minY) y = minY;
        if (y > maxY) y = maxY;
    }

    public String toJson() {
        return "{\"x\":"+x+",\"y\":"+y+"}";
    }

}