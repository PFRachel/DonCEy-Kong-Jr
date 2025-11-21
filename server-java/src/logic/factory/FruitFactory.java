package logic.factory;
import java.util.List;

import domain.Liana;
//La fábrica que crea frutas.
import domain.entities.Fruit;
import domain.entities.GameObject;
public class FruitFactory extends GameObjectFactory {
    private int lianaId;
    private float altura;
    private int puntos;

    public FruitFactory(List<Liana> lianas, int lianaId, float altura, int puntos) {
        super(lianas);
        this.lianaId = lianaId;
        this.altura = altura;
        this.puntos = puntos;
    }

    @Override
    public GameObject create() {
        Liana useLiana = null;
        for (Liana liana : lianas) {
            if (liana.getId() == lianaId) {
                useLiana = liana;
                break;
            }
        }

        float x = useLiana.getXPosition();
        float y = useLiana.getHeadY() + (altura / 100.0f) * (useLiana.getBottomY() - useLiana.getHeadY());
        
        return new Fruit(lianaId, x, y, puntos, lianas);
    }
}
