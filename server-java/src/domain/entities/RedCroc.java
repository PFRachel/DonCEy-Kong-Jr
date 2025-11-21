package domain.entities;

import java.util.List;
import domain.Liana;

//clase concreta que representa un cocodrilo rojo.
public class RedCroc extends  GameObject {
//producto concreto creado por la fábrica de cocodrilos.
    public float vel = 30f;
    private int dir = 1;
    private boolean existe = true;

    private float altura;  // Altura en la liana
    private List<Liana> lianas;

    public RedCroc(int lianaId, float x, float y, List<Liana> lianas) {
        this.liana = lianaId;
        this.x = x;
        this.y = y;
        this.lianas = lianas;
        this.altura = 0;  // Empiezan abajo
        this.dir = 1; // Direccion para arriba

        // Calcular posición inicial
        calcPos();
    }

    @Override
    public int getLiana() {
        return liana;
    }

    @Override
    public String getPantallaDisplay() {
        return pantallaDisplay;
    }

    @Override
    public void setPantallaDisplay(String pantalla) {
        this.pantallaDisplay = pantalla;
    }

    @Override
    public String toJson() {
        return "{\"tipo\":\"rojo\",\"x\":" + x + ","
            + "\"y\":" + y + ","
            + "\"dir\":" + dir + ","
            + "\"pantalla\":\"" + pantallaDisplay + "\""
            + "}";
    }
    
    @Override
    public float getAltura() {
        return y;
    }

    @Override
    public void mover(float dt) {
        // Actualizar altura 
        altura += vel * dir * dt;
        
        // Rebotar en límites
        if (altura >= 100) {
            altura = 100;
            dir = -1;
        } else if (altura <= 0) {
            altura = 0;
            dir = 1;
        }
        
        // Recalcular posición x,y
        calcPos();
    }

    @Override
    public void update(float factor) {
        // Aumentar velocidad 
        this.vel *= factor;
    }

    private void calcPos() {
        if (liana < 0 || liana >= lianas.size()) {
            this.x = 0;
            this.y = 0;
            return;
        }
        
        Liana cliana = lianas.get(liana);
        this.x = cliana.getXPosition();
        
        float rangoY = cliana.getBottomY() - cliana.getHeadY();
        this.y = cliana.getHeadY() + (altura / 100.0f) * rangoY;
    }

    @Override
    // Métodos para que el servidor actualice la posición
    public void setPosition(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public void setDirection(int dir) {
        this.dir = dir;
    }
}
