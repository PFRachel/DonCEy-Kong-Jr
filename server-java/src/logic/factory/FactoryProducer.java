package logic.factory;

import domain.Liana;
import java.util.List;

//El creador de fábricas.
public class FactoryProducer {

            // Para cocodrilos
    public static GameObjectFactory getCrocFactory(String tipo, int lianaId, List<Liana> lianas) {
        return new CrocFactory(tipo, lianaId, lianas);
    }

     // Para frutas
    public static GameObjectFactory getFruitFactory(int lianaId, float altura, int puntos, List<Liana> lianas) {
        return new FruitFactory(lianas, lianaId, altura, puntos);
    }
}
