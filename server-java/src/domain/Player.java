/**
 * ---------------------------------------------------------------
 * Nombre del archivo: Player.java
 * Paquete: domain
 * Descripción:
 *     Representa a un jugador en el sistema del juego Donkey Kong Jr.
 *     Cada jugador tiene un identificador único, puntuación, nivel,
 *     vidas y controla una instancia de Donkey Kong Jr (DKJr).
 *     
 * Responsabilidades:
 *     - Gestionar el estado del jugador (vidas, puntuación, nivel)
 *     - Proporcionar acceso al personaje DKJr asociado
 *     - Modificar atributos del jugador mediante métodos controlados
 * ---------------------------------------------------------------
 */

package domain;
import java.util.List;

public class Player {
    private final int playerId; // 1 o 2
    private int score = 0;
    private int level = 1;
    private int lives = 3;

    private final DKJr personaje; // asociar mono
    private String pantallaId;

    public Player(int playerId, List<Liana> lianas,  List<Plataforma> plataformas) {
        this.playerId = playerId;
        this.personaje = new DKJr(lianas, plataformas);
    }


    public int getPlayerId() { return playerId; } 
    public int getScore() { return score; }
    public int getLevel() { return level; }
    public int getLives() { return lives; }

    public DKJr getMono() { return personaje; }
    public String getPantallaId() { return pantallaId; }

    public void setPantallaId(String pantallaId) {
        this.pantallaId = pantallaId;
    }
    
    public void addScore(int points) {
        score += points;
    }

    public void nextLevel() {
        level++;
    }

    public void killPlayer() {
        chanceLives(-1);
        personaje.resetPos();
    }

    public void chanceLives(int live) {
        lives += live;
    }

    public String toJson() {
        return String.format("{\"id\":%d,\"score\":%d,\"lives\":%d,\"level\":%d,%s}",
        playerId, score, lives, level, personaje.toJson() 
    );
}

}