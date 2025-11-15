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

    import java.util.ArrayList;
    import java.util.List;

    public class DKJr {
        private float x = 100;
        private float y = 650;
        private float velocityX = 0;
        private float velocityY = 0;
        private float gravity = 500f;
        private float jump = -350f;
        private float speed = 100f;

        private boolean onLiana = false;
        private Liana currentLiana = null;
        // Lista de lianas
        private List<Liana> lianas;
        // Pos del Mono en la liana
        private boolean leftSideOfLiana = true;
        private final float lianaOffset = 30f;
        private boolean isAlive = true;
        private boolean isJumping = false;
        private boolean isPlatform = false;
        

        private float minX = 0, maxX = 830;
        private float minY = 0, maxY = 700;

        public DKJr(List<Liana> lianas) {
            this.lianas = lianas;
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

            float move = speed * dt;

            if (onLiana && currentLiana != null) {
                // Movimiento en liana
                movementLiana(up, down, jump,  left, right, move);
            } else {
                // Movimiento libre
                verificarPlataforma();
                platformMovement(up, down, left, right, jump, dt);
                checkLianaGrab();
            }
            
            verificarLimites(); 
    }

        private void movementLiana(int up, int down, int jump,  int left, int right, float dt) {
            float move = 5 * dt;
            
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
                velocityY = jump;
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
            // Aplicar gravedad si no está en liana
            velocityY += gravity * dt;

            velocityX = 0;
            if (right == 1) velocityX = speed;
            if (left == 1) velocityX = -speed;

            // Movimiento horizontal con salto
            if (jump == 1 && isPlatform && !isJumping) {
                velocityY = this.jump;  
                isJumping = true;
                isPlatform = false;
            }

            x += velocityX * dt;
            y += velocityY * dt;
        }

        private void verificarPlataforma() {
            // Si toca el límite inferior
            if (y >= maxY) {
                y = maxY;
                isPlatform = true;
                isJumping = false;
                velocityY = 0;
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
            if (y > maxY) {
                y = maxY;
                isPlatform = true; 
                isJumping = false; 
                velocityY = 0;
            }
            
            // Techo
            if (y < minY) {
                y = minY;
                velocityY = 0;
            }
        }

        private void addLiana(float x, float minY, float maxY) {
            int id = lianas.size(); // ID automático
            lianas.add(new Liana(id, x, minY, maxY));
        }

        public String toJson() {
            return String.format("{\"x\":%.1f,\"y\":%.1f,\"onLiana\":%b,\"jumping\":%b}",
                    x, y, onLiana,  isJumping);
        }

    }