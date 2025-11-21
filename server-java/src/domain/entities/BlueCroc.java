
package domain.entities;

import java.util.List;

import domain.Liana;

//producto concreto, también creado por la misma fábrica (CrocFactory).
public class BlueCroc extends  GameObject {
    private float vel;
    private float altura;
    private final List<Liana> lianas;

    public BlueCroc(int lianaId, float x, float y, List<Liana> lianas) {
        this.liana = lianaId;
        this.x = x;
        this.y = y;
        this.lianas = lianas;
        this.altura = 0; // Empieza abajo
        this.vel = 30f;
        calcPos();
    }

    @Override
    public void mover(float deltaTime) {
        // Movimiento descendente
        altura -= vel * deltaTime;
        calcPos();
    }

    @Override
    public void update(float factor) {
        // Aumentar velocidad
        this.vel *= factor;
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
        return String.format(
            "{\"tipo\":\"azul\",\"x\":%.1f,\"y\":%.1f,\"pantalla\":\"%s\"}",
            x, y, pantallaDisplay
        );
    }

    @Override
    public float getAltura() {
        return 0f;
    }    
    
    private void calcPos() {
        if (liana < 0 || liana >= lianas.size()) return;
        
        Liana l = lianas.get(liana);
        this.x = l.getXPosition();
        float rangoY = l.getBottomY() - l.getHeadY();
        this.y = l.getHeadY() + (altura / 100.0f) * rangoY;
    }
}
