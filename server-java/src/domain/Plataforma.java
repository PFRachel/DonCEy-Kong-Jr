package domain;
 /* Entrada:
 *     - x, y: posición
 *     - width, height: tamaño
 *
 * Salida:
 *     - Plataforma utilizable para colisiones
 *
 * Restricciones:
 *     - No se mueve
 *     - Solo colisiones simples rectangulares
 * ---------------------------------------------------------------
 */
import java.util.Vector;

public class Plataforma {
    private final float x, y; //Posicion
    private final float width, height; //Tamaño

    public Plataforma(float x, float y, float width, float height){
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    // Getters
    public float getX() { return x; }
    public float getY() { return y; }
    public float getWidth() { return width; }
    public float getHeight() { return height; }

    // Verficar si esta sobre una plataforma
    // Toma la posicion del player y retorna T/F
    public boolean onPlataforma(float px, float py) {
        // Tolerancia horizontal 
        float tolerance = 50; 
        
        boolean xRango = (px + tolerance >= x) && (px - tolerance <= x + width);
        boolean yRango = Math.abs(py - y) < 50; // Tolerancia vertical 
        
        return xRango && yRango;
    }

    // Detecta caida del juegador a una plataforma
    public boolean canLandOn(float px, float py, float previousY) {
        float toleranceX = 50;
        float toleranceY = 10; // margen vertical pequeño

        boolean inXRange =
                (px + toleranceX >= x) &&
                (px - toleranceX <= x + width);

        // detectar si viene cayendo
        boolean isFalling = (py > previousY);

        // detectar si está justo sobre la plataforma
        boolean isNearTop = Math.abs(py - y) <= toleranceY;

        return inXRange && isFalling && isNearTop;
    }

    // Techo
    public boolean hitsHead(float px, float py, float playerHeight) {
        float tolerance = 40;
        float playerTop = py - playerHeight;
        float platformBottom = y + height;
        
        boolean inXRange = (px + tolerance >= x) && (px - tolerance <= x + width);
        boolean hitsBottom = (playerTop <= platformBottom) && (playerTop >= y);
        
        return inXRange && hitsBottom;
    }

    public String toJson() {
        return String.format("{\"x\":%.1f,\"y\":%.1f,\"width\":%.1f,\"height\":%.1f}",
            x, y, width, height);
    }
}