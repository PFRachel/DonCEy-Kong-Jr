package domain.entities;

import java.util.List;
import domain.Liana;

//producto concreto creado por la FruitFactory.
public class Fruit extends  GameObject {
//fruta con puntos 
    private float altura;
    private final int puntos;
    private boolean recogido = false;

    public Fruit(int lianaId, float x, float y, int puntos, List<Liana> lianas) {
        this.liana = lianaId;
        this.x = x;
        this.y = y;
        this.puntos = puntos;
    }
    
    @Override
    public String toJson() {
        return "{\"tipo\":\"FRUTA\",\"x\":" + x + 
               ",\"y\":" + y + ",\"pantalla\":\"" + pantallaDisplay + "\"}";
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
    public float getAltura() {
        return altura;
    }
    
    public int getPuntos() {
        return puntos;
    }

    public boolean isRecogido() {
        return recogido;
    }

    public void setRecogido(boolean collected) {
        this.recogido = collected;
    }
}
