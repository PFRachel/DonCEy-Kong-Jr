package logic.factory;
//fábrica que crea cocodrilos.

import domain.entities.*;
import domain.entities.BlueCroc;
import domain.entities.RedCroc;
import domain.entities.GameObject;

public class CrocFactory extends GameObjectFactory {

    @Override
    public GameObject create(String tipo, int liana, float velocidad) {

        switch (tipo.toLowerCase()) {
            case "rojo":
            case "red":
                return new RedCroc(liana, velocidad);

            case "azul":
            case "blue":
                return new BlueCroc(liana, velocidad);
        }

        throw new IllegalArgumentException("Tipo de cocodrilo no válido: " + tipo);
    }
}
