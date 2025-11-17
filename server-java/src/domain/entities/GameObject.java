package domain.entities;
//“todo objeto del juego 
//debe poder serializarse a JSON”
//producto abstracto base.
/**
 * Clase base abstracta para cualquier objeto del juego creado
 * mediante Abstract Factory (cocodrilos, frutas, etc).
 */
public abstract class GameObject {

    protected String tipo;              // "ROJO", "AZUL", "FRUTA"
    protected int liana;                // número de la liana
    protected float altura;             // posición vertical
    protected float velocidad;          // velocidad para crocs
    protected String pantallaObjetivo;  // pantalla1, pantalla2 ...

    public GameObject(String tipo, int liana, float altura, float velocidad) {
        this.tipo = tipo;
        this.liana = liana;
        this.altura = altura;
        this.velocidad = velocidad;
    }

    // -----------------------------
    // GETTERS Y SETTERS NECESARIOS
    // -----------------------------
    public String getTipo() {
        return tipo;
    }

    public int getLiana() {
        return liana;
    }

    public float getAltura() {
        return altura;
    }

    public void setPantallaObjetivo(String p) {
        this.pantallaObjetivo = p;
    }

    public String getPantallaObjetivo() {
        return pantallaObjetivo;
    }

    // -----------------------------
    // MÉTODO ABSTRACTO UPDATE
    // -----------------------------
    public abstract void update(float dt);

    // -----------------------------
    // JSON PARA ENVIAR AL CLIENTE
    // -----------------------------
    public abstract String toJson();
}

