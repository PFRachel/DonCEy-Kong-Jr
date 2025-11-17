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


public class GameState {
    // Límites de jugadores y espectadores según el enunciado
    private static final int MAX_JUGADORES = 4;
    private static final int MAX_ESPECTADORES = 4; // 4
    // Listas ordenadas de conexiones
    private final List<ClientHandler> jugadores = new ArrayList<>();
    private final List<ClientHandler> espectadores = new ArrayList<>();


    // listas «conceptuales»
    private final List<Croc> crocs = new ArrayList<>();
    private final List<Fruit> fruits = new ArrayList<>();
    private final List<Liana> lianas = new ArrayList<>();
    private final Map<String, Player> players = new LinkedHashMap<>();
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

    /**
     * Intenta registrar un nuevo jugador.
     * @return true si se registró, false si ya hay 2 jugadores.
     */
    public synchronized boolean addPlayer(String nick, ClientHandler handler) {
        if (jugadores.size() >= MAX_JUGADORES) {
            return false;
        }
        jugadores.add(handler);
        // Instanciar el Player y añadirlo al mapa
        
        int playerId = players.size() + 1;
        players.put("Player" + playerId, new Player(playerId, lianas));
        System.out.println("[GameState] Jugador añadido: " + nick + " (" + jugadores.size() + "/" + MAX_JUGADORES + ")");
        return true;
    }

    /**
     * Intenta registrar un nuevo espectador.
     * @return true si se registró, false si ya hay 4 espectadores.
     */
    public synchronized boolean addSpectator(String nick, ClientHandler handler) {
        if (espectadores.size() >= MAX_ESPECTADORES) {
            return false;
        }
        espectadores.add(handler);
        System.out.println("[GameState] Espectador añadido: " + nick + " (" + espectadores.size() + "/" + MAX_ESPECTADORES + ")");
        return true;
    }

    public void enqueueInput(String line) {
        inputQueue.add(line);
    }


    
    public void adminSpawnCroc(String line) {
        // "ADMIN_SPAWN_CROC Azul 3 1.5"
        String[] p = line.split("\\s+");
        String tipo = p[1]; 
        int liana = Integer.parseInt(p[2]); 
        float vel = Float.parseFloat(p[3]);
        crocs.add(new Croc(tipo, liana, vel));
    }

    public void adminSpawnFruit(String line) {
        String[] p = line.split("\\s+");
        int liana = Integer.parseInt(p[1]); 
        float altura = Float.parseFloat(p[2]); 
        int puntos = Integer.parseInt(p[3]);
        fruits.add(new Fruit(liana, altura, puntos));
    }

    public void adminDeleteFruit(String line) {
        String[] p = line.split("\\s+");
        int liana = Integer.parseInt(p[1]); float altura = Float.parseFloat(p[2]);
        fruits.removeIf(f -> f.lianaId == liana && Math.abs(f.altura - altura) < 1e-3);
    }

    // ---- bucle de juego ----
    public void update(float dt) {
        tick++;

        // 1) consumir inputs
        String msg;
        while ((msg = inputQueue.poll()) != null) {
            processInput(msg, dt);
        }

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

            if (!players.isEmpty()) {
                Player firstPlayer = players.values().iterator().next();
                firstPlayer.getMono().applyInput(up, down, left, right, jump, dt);
            }
        } catch (Exception e) {
            System.err.println("Error procesando input: " + msg);
            e.printStackTrace();
        }
    }

    public String toJson() {
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
    // -----------------------
    // CLASES DE ENTIDADES (POJO)
    // -----------------------

    // ---- modelos «POJO» mínimos ----
    static class Croc {
        String tipo; int lianaId; float vel; float altura=0; int dir=1;
        Croc(String t,int l,float v){tipo=t; lianaId=l; vel=v;}
        void update(float dt){
            if("Rojo".equals(tipo)){ altura += dir*vel*dt; if(altura>100||altura<0) dir*=-1; }
            else { altura -= vel*dt; /* si pasa <0, cae... */ }
        }
        String toJson(){ return "{\"tipo\":\""+tipo+"\",\"liana\":"+lianaId+",\"altura\":"+altura+"}"; }
    }
    static class Fruit {
        int lianaId; float altura; int puntos;
        Fruit(int l,float a,int p){lianaId=l; altura=a; puntos=p;}
        String toJson(){ return "{\"liana\":"+lianaId+",\"altura\":"+altura+",\"puntos\":"+puntos+"}"; }
    }
}