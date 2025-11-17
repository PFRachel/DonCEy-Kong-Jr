package logic.factory;
//La fábrica que crea frutas.
import domain.entities.Fruit;
import domain.entities.GameObject;

public class FruitFactory implements GameObjectFactory {

    @Override
    public GameObject create(String... p) {

        int liana = Integer.parseInt(p[0]);
        float altura = Float.parseFloat(p[1]);
        int puntos = Integer.parseInt(p[2]);

        return new Fruit(liana, altura, puntos);
    }
}
