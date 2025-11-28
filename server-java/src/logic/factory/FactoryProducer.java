package logic.factory;
/* Entrada:
 *     - tipo: String que indica el tipo de cocodrilo solicitado
 *     - lianaId: índice de la liana donde aparecerá el objeto
 *     - altura: altura inicial para frutas
 *     - puntos: valor en puntos para la fruta
 *     - lianas: lista completa de lianas disponibles en la escena
 *
 * Salida:
 *     - Instancias de GameObjectFactory especializadas:
 *          * CrocFactory
 *          * FruitFactory
 *
 * Restricciones:
 *     - Los parámetros deben ser válidos según lo que exijan
 *       las fábricas específicas.
 *     - No se valida la existencia del tipo solicitado aquí;
 *       cada fábrica es responsable de su validación interna.
 */
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
