package logic.factory;
//La fábrica que crea frutas.
import domain.entities.Fruit;
import domain.entities.GameObject;
public class FruitFactory extends GameObjectFactory {

    @Override
    public GameObject create(int liana, float altura, int puntos) {
        return new Fruit(liana, altura, puntos);
    }
}
