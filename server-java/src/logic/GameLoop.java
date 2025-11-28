/**
 * ---------------------------------------------------------------
 *  Nombre del archivo: GameLoop.java
 *  Paquete: logic
 *  Descripción:
 *      Contiene el bucle principal del juego. Se encarga de
 *      actualizar el estado del juego en intervalos regulares
 *      y notificar a los clientes conectados sobre los cambios.
 * 
 * 
 *      Bucle principal del juego. Ejecuta la lógica del servidor
 *           y envía el estado actualizado en JSON.
 *
 *  Entrada:
 *      - GameState state: estado del juego a actualizar.
 *      - Consumer<String> onStateJson: callback que recibe el JSON.
 *
 *  Salida:
 *      - Envía periódicamente un JSON con el estado del juego.
 *
 *  Restricciones:
 *      - Corre en un único hilo programado (ScheduledExecutor).
 *      - Debe ejecutarse a 60 Hz constantes.
 * ---------------------------------------------------------------
 */
package logic;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

public class GameLoop {
    private final GameState state;
    private final Consumer<String> onStateJson;
    private final ScheduledExecutorService ses = Executors.newSingleThreadScheduledExecutor();

    public GameLoop(GameState state, Consumer<String> onStateJson) {
        this.state = state; this.onStateJson = onStateJson;
    }

    public void start() {
        final long dtMs = 1000 / 60; // 60 Hz
        ses.scheduleAtFixedRate(() -> {
            state.update(1.0f/60.0f);
            onStateJson.accept(state.toJson());
        }, dtMs, dtMs, TimeUnit.MILLISECONDS);
    }

    public void stop() { ses.shutdownNow(); }
}
