
//      Representa una fruta del juego, generada por la FruitFactory.  
//      Las frutas otorgan puntos al jugador y pueden ser recogidas.  
//      Su posición es fija 
//
// Entradas:
//      - int lianaId: índice de la liana a la que pertenece la fruta (opcional).
//      - float x, y: coordenadas iniciales.
//      - int puntos: valor en puntos que otorga la fruta al ser recogida.
//      - List<Liana> lianas: referencia a las lianas del nivel (puede no usarse).
//
// Salidas:
//      - toJson(): representación JSON enviada al cliente C.
//      - Información sobre si la fruta fue o no recogida.
//
// Restricciones:
//      - La fruta no se mueve, solo se dibuja y puede ser marcada como recogida.  
//      - lianaId debe ser un valor válido si la fruta usa la referencia a Liana.  
//      - puntos debe ser mayor a 0.
// ----------------------------------------------------------------------------
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
