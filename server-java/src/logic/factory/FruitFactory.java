package logic.factory;
import java.util.List;

 /* Entrada:
 *     - lianas: lista completa de lianas disponibles
 *     - lianaId: identificador de la liana donde se colocará la fruta
 *     - altura: porcentaje (0–100) indicando la posición vertical
 *               de la fruta dentro del rango de la liana
 *     - puntos: cantidad de puntos otorgados al recolectar la fruta
 *
 * Salida:
 *     - Objeto Fruit (como GameObject) con:
 *          * posición X basada en la liana
 *          * posición Y calculada según el porcentaje de altura
 *          * valor en puntos configurado
 *
 * Restricciones:
 *     - La liana solicitada debe existir en la lista de lianas.
 *     - La altura debe estar en el rango 0–100.
 *     - No se valida que puntos sea positivo; se espera que la lógica
 *       superior provea valores coherentes.
 */
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
        // Rango de altura en la liana 0-100
        float rangoY = useLiana.getBottomY() - useLiana.getHeadY();
        float y = useLiana.getHeadY() + (altura / 100.0f) * rangoY;
        return new Fruit(lianaId, x, y, puntos, lianas);
    }
}
