/**
 * ---------------------------------------------------------------
 *  Nombre del archivo: GameState.java
 *  Paquete: logic
 *  Descripción:
 *      Mantiene el estado global del juego, incluyendo jugadores,
 *      espectadores y entidades del mundo (frutas, cocodrilos, etc.).
 *      Provee métodos para modificar el estado según las acciones
 *      recibidas de los clientes.
 * ---------------------------------------------------------------
 */
package logic;

import domain.DKJr;
import domain.Liana;
import domain.Player;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;

import net.ClientHandler;

// -------------------------
// ABSTRACT FACTORY IMPORTS
// -------------------------
import logic.factory.CrocFactory;
import logic.factory.FruitFactory;
import logic.factory.GameObjectFactory;
import domain.entities.GameObject;


public class GameState {
    // Cada cliente recibe solo su pantalla (pantalla1, pantalla2…)
    private final Map<String, ClientHandler> pantallas = new HashMap<>();

    // Límites de jugadores y espectadores según el enunciado
    private static final int MAX_JUGADORES = 2;
    private static final int MAX_ESPECTADORES = 4; // 4
    // Listas ordenadas de conexiones
    private final List<ClientHandler> jugadores = new ArrayList<>();
    private final List<ClientHandler> espectadores = new ArrayList<>();

    //Lista de jugadores lógicos
    private final Map<String, Player> players = new LinkedHashMap<>();
    
    // listas «conceptuales»
    private final List<GameObject> objetos = new ArrayList<>();

    private final List<Liana> lianas = new ArrayList<>();
    private final ConcurrentLinkedQueue<String> inputQueue = new ConcurrentLinkedQueue<>();
    private float velocidadFactor = 1.0f;
    private long tick = 0;

    public GameState() {
        // Configuración inicial del mapa (6 lianas)
        lianas.add(new Liana(0, 30, 260, 650));
        lianas.add(new Liana(1, 100, 260, 610));
        lianas.add(new Liana(2, 200, 440, 655));
        lianas.add(new Liana(3, 280, 250, 500));
        lianas.add(new Liana(4, 350, 250, 430));

        // AJUSTAR POSICION
        lianas.add(new Liana(5, 500, 120, 650));
        lianas.add(new Liana(6, 600, 400, 600));
        lianas.add(new Liana(7, 700, 50, 400));
        lianas.add(new Liana(8, 750, 120, 650));
    
    }

    // -----------------------
    // MÉTODOS DE REGISTRO (red)
    // -----------------------
    public synchronized boolean addPlayer(String nick, ClientHandler handler) {

        if (jugadores.size() >= MAX_JUGADORES) return false;

        String pantalla = "pantalla" + (jugadores.size() + 1);
        pantallas.put(pantalla, handler);

        handler.setPantallaId(pantalla);
        handler.setTipo("PLAYER");

        jugadores.add(handler);

        // Crear jugador lógico
        int playerId = players.size() + 1;
        players.put("Player" + playerId, new Player(playerId, lianas));

        System.out.println("[GameState] Jugador " + nick + " asignado a " + pantalla);
        return true;
    }

    public synchronized boolean addSpectator(String nick, ClientHandler handler) {

        if (espectadores.size() >= MAX_ESPECTADORES) return false;

        String pantalla = "pantalla" + (espectadores.size() + 1);
        pantallas.put(pantalla, handler);

        handler.setPantallaId(pantalla);
        handler.setTipo("SPECTATOR");

        espectadores.add(handler);

        System.out.println("[GameState] Espectador " + nick + " asignado a " + pantalla);
        return true;
    }
    //cola de imputs 
     public void enqueueInput(String msg) {
        inputQueue.add(msg);
    }
    // COMANDOS DEL ADMIN: ABSTRACT FACTORY
    // ==========================================================

    // Ejemplo: ADMIN_SPAWN_CROC rojo 3 1.5 pantalla2
    public void adminSpawnCroc(String msg) {

        String[] p = msg.split("\\s+");
        if (p.length < 5) {
            System.out.println("[ADMIN] Formato inválido: ADMIN_SPAWN_CROC tipo liana vel pantalla");
            return;
        }

        String tipo = p[1];
        int liana = Integer.parseInt(p[2]);
        float vel = Float.parseFloat(p[3]);
        String pantalla = p[4];

        GameObjectFactory factory = new CrocFactory();
        GameObject croc = factory.create(tipo, liana, vel);

        croc.setPantallaObjetivo(pantalla);
        objetos.add(croc);

        System.out.println("[ADMIN] Cocodrilo creado en " + pantalla);
    }

    // Ejemplo: ADMIN_SPAWN_FRUIT 2 70 300 pantalla1
    public void adminSpawnFruit(String msg) {

        String[] p = msg.split("\\s+");
        if (p.length < 5) {
            System.out.println("[ADMIN] Formato inválido: ADMIN_SPAWN_FRUIT liana altura puntos pantalla");
            return;
        }

        int liana = Integer.parseInt(p[1]);
        float altura = Float.parseFloat(p[2]);
        int puntos = Integer.parseInt(p[3]);
        String pantalla = p[4];

        GameObjectFactory factory = new FruitFactory();
        GameObject fruit = factory.create(liana, altura, puntos);

        fruit.setPantallaObjetivo(pantalla);
        objetos.add(fruit);

        System.out.println("[ADMIN] Fruta creada en " + pantalla);
    }

    // Ejemplo: ADMIN_DELETE_FRUIT 2 70 pantalla1
    public void adminDeleteFruit(String msg) {

        String[] p = msg.split("\\s+");
        if (p.length < 4) return;

        int liana = Integer.parseInt(p[1]);
        float altura = Float.parseFloat(p[2]);
        String pantalla = p[3];

        objetos.removeIf(o ->
                o.getTipo().equals("FRUTA") &&
                o.getLiana() == liana &&
                Math.abs(o.getAltura() - altura) < 1e-3 &&
                pantalla.equals(o.getPantallaObjetivo())
        );

        System.out.println("[ADMIN] Fruta eliminada de " + pantalla);
    }
    // ---- bucle de juego ----
    public void update(float dt) {
        tick++;

        // 1) consumir inputs
        String msg;
        while ((msg = inputQueue.poll()) != null) {
            processInput(msg, dt);
        }
        // 2. Actualizar objetos Abstract Factory
        for (GameObject obj : objetos) obj.update(dt * velocidadFactor);

        // 2) mover cocodrilos
        //for (Croc c : crocs) c.update(dt * velocidadFactor);

        // 3) colisiones (simplificado)
        // ... detectar y actualizar vidas/puntaje

        // 4) reglas de rescate → subir dificultad
        // if (rescato) { dkjr.vidas++; velocidadFactor *= 1.10f; reiniciarNivel(); }
    }

    private void processInput(String msg, float dt) {
        try {
            // Formato: "INPUT <tick> <up> <down> <left> <right> <jump>"
            String[] p = msg.split("\\s+");
            if (p.length < 7) return;
            
            int up = Integer.parseInt(p[2]);
            int down = Integer.parseInt(p[3]);
            int left = Integer.parseInt(p[4]);
            int right = Integer.parseInt(p[5]);
            int jump = Integer.parseInt(p[6]);

            if (players.isEmpty()) return;

            Player firstPlayer = players.values().iterator().next();
            firstPlayer.getMono().applyInput(up, down, left, right, jump, dt);
        } catch (Exception e) {
            System.err.println("Error procesando input: " + msg);
            e.printStackTrace();
        }
    }

    public String toJson(String pantalla) {
        // JSON sencillo a mano (puedes cambiar a una lib luego)
        StringBuilder sb = new StringBuilder();
        sb.append("{\"tick\":").append(tick);
        if (!players.isEmpty()) {
            Player firstPlayer = players.values().iterator().next();
            sb.append(",\"dkjr\":").append(firstPlayer.getMono().toJson());
        } else {
            sb.append(",\"dkjr\":null");
        }
        sb.append(",\"lianas\":[");
        if (!players.isEmpty()) {
            Player firstPlayer = players.values().iterator().next();
            List<Liana> lianas = firstPlayer.getMono().getLianas();
            for (int i = 0; i < lianas.size(); i++) {
                if (i > 0) sb.append(",");
                sb.append(lianas.get(i).toJson());
            }
        }
        // Enviar SOLO objetos de esta pantalla
        sb.append(",\"objetos\":[");
        boolean first = true;
        for (GameObject o : objetos) {
            if (!pantalla.equals(o.getPantallaObjetivo())) continue;

            if (!first) sb.append(",");
            sb.append(o.toJson());
            first = false;
        }
        //for (int i = 0; i < crocs.size(); i++) {
          //  if (i>0) sb.append(",");
            //sb.append(crocs.get(i).toJson());
        //}
        //sb.append("],\"fruits\":[");
        //for (int i = 0; i < fruits.size(); i++) {
          //  if (i>0) sb.append(",");
           // sb.append(fruits.get(i).toJson());
        //}
        sb.append("]}");
        return sb.toString();
    }
    
}