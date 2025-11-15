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

    private DKJr personaje; // asociar mono

    public Player(int playerId, List<Liana> lianas ) {
        this.playerId = playerId;
        this.personaje = new DKJr(lianas);
    }


    public int getPlayerId() { return playerId; }
    public int getScore() { return score; }
    public int getLevel() { return level; }
    public int getLives() { return lives; }

    public DKJr getMono() { return personaje; }

    public void addScore(int points) {
        score += points;
    }

    public void nextLevel() {
        level++;
    }

    public void chanceLives(int live) {
        lives += live;
    }
}