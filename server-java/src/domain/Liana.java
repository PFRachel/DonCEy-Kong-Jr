package domain;

public class Liana {
    private final int id;
    private final float xPosition;
    private final float headY;
    private final float bottomY;
    
    public Liana(int id, float xPosition, float headY, float bottomY) {
        this.id = id;
        this.xPosition = xPosition;
        this.headY = headY;
        this.bottomY = bottomY;
    }
    
    // Getters
    public int getId() { return id; }
    public float getXPosition() { return xPosition; }
    public float getHeadY() { return headY; }
    public float getBottomY() { return bottomY; }
    
    // Verificar si lo puede agarrar
    public boolean canGrab(float playerX, float playerY) {
        float marginX = 30f; // ancho horizontal de la liana
        return playerY >= headY && playerY <= bottomY && Math.abs(playerX - xPosition) <= marginX;
    }
    
    // Verificar que no este por encima del la liana o abajo
    public boolean inRangoY(float playerY) {
        return playerY >= headY && playerY <= bottomY;
    }
    
    public String toJson() {
        return String.format("{\"id\":%d,\"x\":%.1f,\"topY\":%.1f,\"bottomY\":%.1f}",
                id, xPosition, headY, bottomY);
    }
    
}