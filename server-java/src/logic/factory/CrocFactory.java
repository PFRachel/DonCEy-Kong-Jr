package logic.factory;
//fábrica que crea cocodrilos.
 /* Descripción:
 *     Fábrica concreta encargada de crear cocodrilos del tipo
 *     solicitado (rojo o azul). Implementa el patrón Abstract Factory
 *     extendiendo GameObjectFactory.
 *
 * Entrada:
 *     - tipo: String que indica el tipo de cocodrilo ("red" o "blue")
 *     - lianaId: índice de la liana donde aparecerá el cocodrilo
 *     - lianas: lista completa de lianas disponibles en la escena
 *
 * Salida:
 *     - Instancia de GameObject (RedCroc o BlueCroc) ya posicionada
 *       en la liana indicada.
 *
 * Restricciones:
 *     - tipo debe ser "red" o "blue" (case-insensitive)
 *     - lianaId debe estar dentro del rango de la lista de lianas
 *     - La fábrica no crea objetos si el tipo es inválido
 */
import java.util.List;

import domain.Liana;
import domain.entities.BlueCroc;
import domain.entities.RedCroc;
import domain.entities.GameObject;

// Factory para crear cocodrilos
public class CrocFactory extends GameObjectFactory{

    private String tipo;
    private int lianaId;
    
    public CrocFactory(String tipo, int lianaId, List<Liana> lianas) {
        super(lianas); // Constructor del padre
        this.tipo = tipo;
        this.lianaId = lianaId;

        Liana liana = lianas.get(lianaId);
        float x = liana.getXPosition();
        float y = liana.getHeadY();
    }

    @Override
    public GameObject create() {
        Liana liana = lianas.get(lianaId);
        float x = liana.getXPosition();
        float y = liana.getHeadY();
        
        switch (tipo.toLowerCase()) {
            case "red":
                //System.out.println("[CrocFactory] Cocodrilo ROJO creado");
                return new RedCroc(lianaId, x, y, lianas);

            case "blue":
                //System.out.println("[CrocFactory] Cocodrilo AZUL creado");
                return new BlueCroc(lianaId, x, y, lianas);

            default:
                throw new IllegalArgumentException("Tipo de cocodrilo no soportado: " + tipo);
        }
    }
}
