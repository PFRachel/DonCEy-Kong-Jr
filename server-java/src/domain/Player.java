package domain;

public class Player {
    private final int playerId; // 1 o 2
    private int score = 0;
    private int level = 1;
    private int lives = 3;

    private DKJr personaje; // asociar mono

    public Player(int playerId) {
        this.playerId = playerId;
        this.personaje = new DKJr();
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