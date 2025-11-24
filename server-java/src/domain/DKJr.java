/**
 * ---------------------------------------------------------------
 * Nombre del archivo: DKJr.java
 * Paquete: domain
 * Descripción:
 *     Representa al personaje principal Donkey Kong Jr en el juego.
 *     Controla la posición, movimiento, estado físico y interacciones
 *     con el entorno (lianas, plataformas, enemigos).
 *     
 * Responsabilidades:
 *     - Gestionar posición y movimiento del personaje
 *     - Aplicar inputs del jugador al estado del personaje
 *     - Controlar límites de movimiento y colisiones básicas
 *     - Manejar estados especiales (en lianas, saltando, etc.)
 * ---------------------------------------------------------------
 */
package domain;

import java.util.List;

public class DKJr {
    // Posición inicial
    private final float startX = 100;
    private final float startY = 700;
    //Pos actual
    private float x = startX;
    private float y = startY;
    private float velocityX = 0;
    private float velocityY = 0;
    private final float gravity = 500f;
    private final float jump = -350f;
    private final float speed = 100f;

    private boolean onLiana = false;
    private Liana currentLiana = null;
    // Lista de lianas
    private final List<Liana> lianas;
    // Pos del Mono en la liana
    private boolean leftSideOfLiana = true;
    private final float lianaOffset = 30f;
    private boolean isAlive = true;
    private boolean isJumping = false;
    // Lista de plataformas
    private final List<Plataforma> plataformas;
    private boolean onPlatform = false;
    

    private final float minX = 0, maxX = 830;
    private final float minY = 0, maxY = 870;

    public DKJr(List<Liana> lianas, List<Plataforma> plataformas) {
        this.lianas = lianas;
        this.plataformas = plataformas;
    }

    public float getX() { return x; }
    public float getY() { return y; }
    public boolean isOnLiana() { return onLiana; }
    public Liana getCurrentLiana() { return currentLiana; }
    public List<Liana> getLianas() { return lianas; }
    public boolean isJumping() { return isJumping; }
    //public boolean isAlive() { return isAlive; }
    
    // Posicion inicial
    public void setPosition(float x, float y) {
        this.x = x;
        this.y = y;
        this.velocityX = 0;
        this.velocityY = 0;
        this.isJumping = false;
    }

    public void applyInput(int up, int down, int left, int right, int jump, float dt) {

        if (onLiana && currentLiana != null) {
            // Movimiento en liana
            movementLiana(up, down, jump,  left, right, dt);
        } else {
            // Movimiento libre
            platformMovement(up, down, left, right, jump, dt);
            checkLianaGrab();
        }
        
        verificarLimites(); 
    }

        private void movementLiana(int up, int down, int jump,  int left, int right, float dt) {
            float move = speed * dt;
            
            // Subir y Bajar
            if (up == 1) y -= move;
            if (down == 1) y += move;
            
            // Cambio lateral de lado
            if (left == 1)  leftSideOfLiana = true;
            if (right == 1) leftSideOfLiana = false;

            x = currentLiana.getXPosition() + (leftSideOfLiana ? -lianaOffset : lianaOffset);

            if (jump == 1) {
                onLiana = false;
                currentLiana = null;
                velocityY = this.jump;
                isJumping = true;
                return;
            }
            // Verificar límites de la liana actual
            if (!currentLiana.inRangoY(y)) {
                // Busca si hay otra liana cercana horizontalmente
                for (Liana l : lianas) {
                    if (l.canGrab(x, y)) {
                        currentLiana = l;
                        return;
                    }
                
                 }
            // No hay liana, empezar a caer
            onLiana = false;
            currentLiana = null;
            isJumping = true;
            velocityY = 0; // caída inicial
        }   
    }

    private void platformMovement(int up, int down, int left, int right, int jump, float dt) {
        float oldY = y; 

        // Aplicar gravedad si no está en liana
        if (!onPlatform) {
            velocityY += gravity * dt;
        }

        velocityX = 0;
        if (right == 1) velocityX = speed;
        if (left == 1) velocityX = -speed;

        // Movimiento horizontal con salto
        if (jump == 1 && onPlatform && !isJumping) {
            velocityY = this.jump;  
            isJumping = true;
            onPlatform = false;
        }

        // Movimiento
        x += velocityX * dt;
        y += velocityY * dt;

        // Reset valor
        onPlatform = false;

        // Aplicar logica para cada plataformma
        for (Plataforma p : plataformas){

            // Impactar caida (ya no se revisa velocityY > 0 aquí)
            if (p.canLandOn(x, y, oldY)){
                y = p.getY();
                velocityY = 0;
                isJumping = false;
                onPlatform = true;
                break;
            } else if (velocityY < 0 && p.hitsHead(x, y, 40)){
                // Solo considerar golpe en la cabeza cuando se mueve hacia arriba
                y = p.getY() + p.getHeight();
                velocityY = 0;
            }
            if (velocityY < 0 && p.hitsHead(x, y, 40)) {
                // Igual que arriba, pero respetando tu estructura original
                y = p.getY() + p.getHeight();
                velocityY = 0;
            }
        }
    }
    
    private void checkLianaGrab() {
        if (velocityY >= 0 || isJumping) {
            for (Liana liana : lianas) {
                if (liana.canGrab(x, y)) {
                    onLiana = true;
                    currentLiana = liana;
                   // x = liana.getXPosition(); // Centrar en liana
                    velocityY = 0; // Detener caída/salto
                    isJumping = false;
                    onPlatform = false;
                    break;
                }
            }
        }
    }

    private void verificarLimites() {
        // Límites horizontales
        if (x < minX) x = minX;
        if (x > maxX) x = maxX;
        
        // Colisión con el suelo
        if (y < minY) {
            y = minY;
            velocityY = 0;
        }
        if (y > maxY) {
            y = maxY;
            velocityY = 0;
        }
    }

    // Posicion inicial 
    public void resetPos() {
        this.x = startX;
        this.y = startY;
        this.velocityX = 0;
        this.velocityY = 0;
        this.onLiana = false;
        this.currentLiana = null;
        this.isJumping = false;
        this.onPlatform = false;
    }

    public String toJson() {
        return String.format("{\"x\":%.1f,\"y\":%.1f,\"onLiana\":%b,\"jumping\":%b}",
                x, y, onLiana,  isJumping);
    }

}