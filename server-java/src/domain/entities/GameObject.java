package domain.entities;
//“todo objeto del juego 
//debe poder serializarse a JSON”
//producto abstracto base.
/**
 * Clase base abstracta para cualquier objeto del juego creado
 * mediante Abstract Factory (cocodrilos, frutas, etc).
 */
public abstract class GameObject {
    protected String pantallaDisplay;
    protected int liana;
    protected float x, y;

    // -----------------------------
    // JSON PARA ENVIAR AL CLIENTE
    // -----------------------------
     public abstract String toJson();

    // -----------------------------
    // INDICE DEL LIANA DONDE ESTA EL OBJETO
    // -----------------------------
    public int getLiana() { return liana; }

    // -----------------------------
    // ALTURA EN LA LIANA
    // -----------------------------
    public float getAltura() { return y; }

    // -----------------------------
    // CUAL CLIENTE
    // -----------------------------
    public String getPantallaDisplay() { return pantallaDisplay; }

    // -----------------------------
    // DIFINIR CLIENTE
    // -----------------------------
    public void setPantallaDisplay(String pantalla) { this.pantallaDisplay = pantalla; }

    public void setPosition(float x, float y) { 
        this.x = x; 
        this.y = y; 
    }

    // Métodos solo para crocs
    public void mover(float deltaTime) {} // Movimiento
    public void update(float factor) {} // Aumentar vel
}

