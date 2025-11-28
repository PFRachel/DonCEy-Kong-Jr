package logic.factory;
//La fábrica abstracta
import domain.entities.GameObject;
import domain.Liana;
import java.util.List;
/* Entrada:
 *     - lianas: lista global de lianas del escenario que las
 *               fábricas pueden necesitar para calcular posiciones.
 *
 * Salida:
 *     - Método abstracto create(), que retorna un objeto GameObject
 *       (cocodrilos, frutas, etc.) creado por una subfábrica concreta.
 *
 * Restricciones:
 *     - La lista de lianas no debe ser nula al crear cualquier
 *       instancia de fábrica.
 *     - Las subclases deben implementar obligatoriamente create().
 */
//CREATOR OBJETOS.
public abstract class GameObjectFactory {

    protected List<Liana> lianas;
    
    public GameObjectFactory(List<Liana> lianas) {
        this.lianas = lianas;
    }

    public abstract GameObject create();
    
}

//Define cómo crean objetos TODAS las fábricas.
