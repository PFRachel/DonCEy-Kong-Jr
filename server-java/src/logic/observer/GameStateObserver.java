package logic.observer;
/* Entrada:
 *     - stateJson: cadena JSON que contiene el estado
 *       completo o parcial del juego.
 *
 * Salida:
 *     - No retorna valores. Se espera que la implementación 
 *       procese el JSON recibido.
 *
 * Restricciones:
 *     - Las clases que implementen esta interfaz deben manejar
 *       correctamente el contenido del JSON.
 *     - El JSON puede representar múltiples entidades del juego.
 */
public interface GameStateObserver {
    void onStateJson(String stateJson);
}

