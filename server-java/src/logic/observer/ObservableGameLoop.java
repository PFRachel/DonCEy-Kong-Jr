package logic.observer;

import logic.GameLoop;
import logic.GameState;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Observable del patrón Observer.
 * Envuelve el GameLoop existente y notifica Observers
 * cada vez que el estado del juego cambia.
 */
public class ObservableGameLoop {

    private final GameState state;
    private final List<GameStateObserver> observers = new CopyOnWriteArrayList<>();
    private final GameLoop loop;

    public ObservableGameLoop(GameState state) {
        this.state = state;

        // Adapter: GameLoop → notifyObservers(JSON)
        this.loop = new GameLoop(state, this::notifyObservers);
    }

    public void addObserver(GameStateObserver obs) {
        if (obs != null) observers.add(obs);
    }

    public void removeObserver(GameStateObserver obs) {
        observers.remove(obs);
    }

    private void notifyObservers(String stateJson) {
        for (GameStateObserver o : observers) {
            o.onStateJson(stateJson);
        }
    }

    public void start() { loop.start(); }
    public void stop() { loop.stop(); }
}