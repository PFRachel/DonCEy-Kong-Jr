package logic.factory;
//La fábrica abstracta
import domain.entities.GameObject;
//CREATOR ABSTRACTO.

public abstract class GameObjectFactory {

    // Para COCODRILOS
    public GameObject create(String tipo, int liana, float velocidad) {
        throw new UnsupportedOperationException("create(tipo,liana,velocidad) no implementado");
    }

    // Para FRUTAS
    public GameObject create(int liana, float altura, int puntos) {
        throw new UnsupportedOperationException("create(liana,altura,puntos) no implementado");
    }
}

//Define cómo crean objetos TODAS las fábricas.
