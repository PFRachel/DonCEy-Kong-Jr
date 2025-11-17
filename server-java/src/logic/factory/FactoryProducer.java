package logic.factory;
//El creador de fábricas.
public class FactoryProducer {

    public static GameObjectFactory getFactory(String type) {

        switch (type.toLowerCase()) {
            case "crocodrilo":
            case "cocodrilo": return new CrocFactory();
            case "fruta":     return new FruitFactory();
        }//la parte “Abstract Factory → Produce Factories”

        throw new IllegalArgumentException("Tipo fábrica no reconocido");
    }
}
