package logic.factory;
//fábrica que crea cocodrilos.

import domain.entities.*;
public class CrocFactory implements GameObjectFactory {

    @Override
    public GameObject create(String... p) {

        String tipo = p[0];   // rojo | azul
        int liana = Integer.parseInt(p[1]);
        float vel = Float.parseFloat(p[2]);

        switch (tipo.toLowerCase()) {
            // crea los objetos RedCroc y BlueCroc
            case "rojo": return new RedCroc(liana, vel);
            case "azul": return new BlueCroc(liana, vel);
            default: throw new IllegalArgumentException("Tipo croc inválido");
        }
    }
}
