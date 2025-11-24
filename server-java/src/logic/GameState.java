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
import domain.Plataforma;
import domain.Player;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;

import net.ClientHandler;

// -------------------------
// ABSTRACT FACTORY IMPORTS
// -------------------------
import logic.factory.FactoryProducer;
import logic.factory.GameObjectFactory;
import domain.entities.BlueCroc;
import domain.entities.GameObject;
import domain.entities.RedCroc;


public class GameState {
    private final Random random = new Random();
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
    private final List<Plataforma> plataformas = new ArrayList<>();
    private final ConcurrentLinkedQueue<String> inputQueue = new ConcurrentLinkedQueue<>();
    private float velocidadFactor = 1.0f;
    private long tick = 0;
    private int level = 1;  // nivel inicial

    // para autospawn de cocodrulos
    private float crocTimer = 0f;   // tiempo acumulado
    private float crocIntervalo = 3.0f;  // crear uno cada 3s
    private int maxCroc = 6; // maximo de crocs

    // Espectadores por jugador (pantalla1 / pantalla2)
    private final Map<ClientHandler, String> spectatorTargets = new HashMap<>();
    private int spectatorsPantalla1 = 0;
    private int spectatorsPantalla2 = 0;

    public GameState() {
        // Configuración inicial del mapa
        // Agregar lianas soloes estas priemras 5 estan bien 
        lianas.add(new Liana(0, 30, 260, 650));// Extremo izquierdo
        lianas.add(new Liana(1, 100, 260, 610));// Segunda desde izquierda
        lianas.add(new Liana(2, 200, 440, 655));// Debajo plataforma media-izquierda
        lianas.add(new Liana(3, 350, 250, 590));// Centro-izquierda
        lianas.add(new Liana(4, 500, 250, 430));
        //lianas.add(new Liana(5, 500, 340, 500)); // extra
        lianas.add(new Liana(5, 570, 340, 530));
        lianas.add(new Liana(6, 640, 340, 480));  // Penúltima
        lianas.add(new Liana(7, 720, 190, 580));  // Última del fondo
        lianas.add(new Liana(8, 760, 190, 580));
        
        // ===== PLATAFORMAS =====
        plataformas.add(new Plataforma(20, 720, 120, 30)); //Plataforma principal inicio
        plataformas.add(new Plataforma(348, 670, 10, 10));// Isla verde 1
        plataformas.add(new Plataforma(493, 705,  10, 10));// Isla verde 2
        plataformas.add(new Plataforma(610, 670, 10, 10));// Isla verde 3
        plataformas.add(new Plataforma(756, 640, 10, 10));// Isla verde 4

        
        plataformas.add(new Plataforma(173, 350, 10, 18));// Plataforma más alta de las dos del centro
        plataformas.add(new Plataforma(178, 500, 40, 10)); // Plataforma izquierda inferior
        plataformas.add(new Plataforma(697, 430, 90, 10));// Plataforma central derecha
        plataformas.add(new Plataforma(522, 185, 10, 10));// desnivel hacia la derecha
        plataformas.add(new Plataforma(260, 117,  40, 5));// Plataforma pequeña sobre Mario
        plataformas.add(new Plataforma(0,   175, 500, 10));// Plataforma grande superior
        // Elementos de Default, fijos
        // Croc(tipo, lianaId, pantallaDisplay)
        adminSpawnCroc("ADMIN_SPAWN_CROC red 0 pantalla1");
        // Fruit(lianaId, altura, puntos, pantallaDisplay)
        adminSpawnFruit("ADMIN_SPAWN_FRUIT 1 10 40 pantalla1");
        adminSpawnFruit("ADMIN_SPAWN_FRUIT 2 10 40 pantalla1");
        adminSpawnFruit("ADMIN_SPAWN_FRUIT 2 90 40 pantalla1");
        adminSpawnFruit("ADMIN_SPAWN_FRUIT 2 90 40 pantalla1");

        adminSpawnFruit("ADMIN_SPAWN_FRUIT 1 10 40 pantalla2");
        adminSpawnFruit("ADMIN_SPAWN_FRUIT 6 10 40 pantalla2");
        adminSpawnFruit("ADMIN_SPAWN_FRUIT 2 90 40 pantalla2");
        adminSpawnFruit("ADMIN_SPAWN_FRUIT 3 90 40 pantalla2");
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

        String pantalla = "pantalla" + (jugadores.size() + 1);
        pantallas.put(pantalla, handler);

        handler.setPantallaId(pantalla);
        handler.setTipo("PLAYER");

        jugadores.add(handler);

        // Crear jugador lógico
        int playerId = players.size() + 1;
        Player newPlayer = new Player(playerId, lianas, plataformas);
        newPlayer.setPantallaId(pantalla);
        players.put("Player" + playerId, newPlayer);

        System.out.println("[GameState] Jugador " + nick + " asignado a " + pantalla);
        return true;
    }

    /**
     * Intenta registrar un nuevo espectador.
     * Reglas:
     *  - Máximo 2 espectadores por pantalla (pantalla1 y pantalla2).
     *  - Máximo 4 espectadores en total.
     *  - Si aún no existe el jugador 2, igual se pueden asignar
     *    espectadores a pantalla2 (verán "NO HAY JUGADOR CONECTADO").
     */
    public synchronized boolean addSpectator(String nick, ClientHandler handler) {

        if (espectadores.size() >= MAX_ESPECTADORES) {
                return false;
        }

        // Decidir a qué pantalla se le asigna este espectador
        // Regla: hasta 2 por pantalla. Primero pantalla1, luego pantalla2.
        String targetPantalla = null;
        if (spectatorsPantalla1 < 2) {
            targetPantalla = "pantalla1";
        } else if (spectatorsPantalla2 < 2) {
            targetPantalla = "pantalla2";
        } else {
            // Ya hay 2 en cada pantalla (4 en total)
            return false;
        }

        String pantalla = "spectator" + (espectadores.size() + 1);
        pantallas.put(pantalla, handler);

        handler.setPantallaId(pantalla);
        handler.setTipo("SPECTATOR");

        espectadores.add(handler);

        // Registrar a qué jugador está asociado este espectador
        spectatorTargets.put(handler, targetPantalla);
        if ("pantalla1".equals(targetPantalla)) {
            spectatorsPantalla1++;
        } else if ("pantalla2".equals(targetPantalla)) {
            spectatorsPantalla2++;
        }

        System.out.println("[GameState] Espectador " + nick + " asignado a " + targetPantalla);
        return true;
    }

    // Devuelve a qué pantalla (pantalla1 / pantalla2) está ligado un espectador
    public synchronized String getSpectatorTargetFor(ClientHandler handler) {
        return spectatorTargets.get(handler);
    }

    //cola de imputs 
    public void enqueueInput(String msg) {
        inputQueue.add(msg);
    }

    // -----------------------
    // DESCONEXIÓN DE CLIENTES
    // -----------------------
    public synchronized void onClientDisconnected(ClientHandler handler) {
        if (handler == null) return;

        String pantalla = handler.getPantallaId();
        String tipo = handler.getTipo();

        if (pantalla == null || tipo == null) return;

        // Quitar del map de pantallas
        pantallas.remove(pantalla);

        if ("PLAYER".equals(tipo)) {
            // Quitar de la lista de jugadores
            jugadores.remove(handler);

            // Quitar el Player lógico asociado a esa pantalla
            players.entrySet().removeIf(e ->
                    pantalla.equals(e.getValue().getPantallaId())
            );

            System.out.println("[GameState] Jugador desconectado de " + pantalla);
        } else if ("SPECTATOR".equals(tipo)) {
            // Quitar de la lista de espectadores
            espectadores.remove(handler);

            // Ajustar contador de espectadores por jugador
            String targetPantalla = spectatorTargets.remove(handler);
            if ("pantalla1".equals(targetPantalla)) {
                if (spectatorsPantalla1 > 0) spectatorsPantalla1--;
            } else if ("pantalla2".equals(targetPantalla)) {
                if (spectatorsPantalla2 > 0) spectatorsPantalla2--;
            }

            System.out.println("[GameState] Espectador desconectado de " + pantalla);
        }
    }

    // Contar cant de crocs
    private int contarCroc(String pantalla) {
    int count = 0;
    for (GameObject o : objetos) {
        if (o instanceof BlueCroc && pantalla.equals(o.getPantallaDisplay())) {
            count++;
        }
    }
        return count;
    }   

    // COMANDOS DEL ADMIN: ABSTRACT FACTORY
    // ==========================================================


    // Ejemplo: ADMIN_SPAWN_CROC rojo 3 1.5 pantalla2
    public void adminSpawnCroc(String msg) {
        String[] p = msg.split("\\s+");
        if (p.length < 4) {
            System.out.println("[ADMIN] Formato inválido: ADMIN_SPAWN_CROC tipo liana vel pantalla");
            return;
        }

        String tipo = p[1];
        int liana = Integer.parseInt(p[2]);
        String pantalla = p[3];
        try {
            GameObjectFactory factory = FactoryProducer.getCrocFactory(tipo, liana, lianas);
            GameObject croc = factory.create();
            croc.setPantallaDisplay(pantalla);
            objetos.add(croc);
            
            System.out.println("[TEST] Objetos creados:");
            for (GameObject obj : objetos) {
                System.out.println(obj.toJson());
            }

        } catch (Exception e) {
            System.err.println("✗ ERROR al crear cocodrilo: " + e.getMessage() + "\n");
        }
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

        GameObjectFactory factory = FactoryProducer.getFruitFactory(liana, altura, puntos, lianas);
        GameObject fruit = factory.create(); 
        fruit.setPantallaDisplay(pantalla);
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
                o.getLiana() == liana &&
                Math.abs(o.getAltura() - altura) < 1e-3 &&
                pantalla.equals(o.getPantallaDisplay())
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

         // 2) mover cocodrilos
        Iterator<GameObject> it = objetos.iterator();
        while (it.hasNext()) {
            GameObject obj = it.next();

            // Llamar metodo mover dentro de cada objeto
            obj.mover(dt * velocidadFactor);

            // Eliminar croc si llega abajo
            if (obj instanceof BlueCroc croc) {
                if (croc.getAltura() >= 100) {
                    it.remove();
                    continue;
                }
            }
        }
            crocTimer += dt;
            if (crocTimer >= crocIntervalo) {
                crocTimer = 0;

                autoSpawnCroc();
            }
                    
            // 2. Actualizar objetos Abstract Factory
            //for (GameObject obj : objetos) obj.update(dt * velocidadFactor);

        
            //for (Croc c : crocs) c.update(dt * velocidadFactor);

            // 3) colisiones 
            // ... detectar y actualizar vidas/puntaje
            detectarcolisiones();

            // 4) reglas de rescate → subir dificultad
            // if (rescato) { dkjr.vidas++; velocidadFactor *= 1.10f; reiniciarNivel(); }
            // --- DETECCIÓN DE META PARA CADA JUGADOR ---
        for (Player p : players.values()) {
            DKJr mono = p.getMono();

            if (haLlegadoMeta(mono)) {

                levelUp(p);  // subir nivel + reset 
            }
        } 


        }

    // Funcion para dirigir spawn automatico de cocodrilos en x pantalla
    private void autoSpawnCroc() {

        // Ajustar tiempo de spawn por nivel
        crocIntervalo = Math.max(0.6f, 2.0f / level);

         // pantalla 1
        if (pantallas.containsKey("pantalla1") && contarCroc("pantalla1") < maxCroc) {
            spawnBlueCroc("pantalla1");
        }

        // pantalla 2
        if (pantallas.containsKey("pantalla2") && contarCroc("pantalla2") < maxCroc) {
            spawnBlueCroc("pantalla2");
        }
    }

    // Crear cocodrilo
    private void spawnBlueCroc(String pantalla) {
        // calcular liana random
        int lianaId = random.nextInt(lianas.size());

        // Crear cocodrilo azul
        GameObjectFactory f = FactoryProducer.getCrocFactory("blue", lianaId, lianas);
        GameObject croc = f.create();

        // Asignar pantalla
        croc.setPantallaDisplay(pantalla);
        
        // Agregar al arreglo
        objetos.add(croc);
        System.out.println("[SPAWN1] BlueCroc → " + pantalla + " (Liana " + lianaId + ")");
    }
    //=====================================
    //========funcion de ha llegado a la meta =======================
    private boolean haLlegadoMeta(DKJr mono) {
    float metaX = 240;
    float metaY = 117;
    float metaW = 80;
    float metaH = 20;
    //plataformas.add(new Plataforma(260, 117,  88, 25));// Plataforma pequeña sobre Mario
    return mono.getX() > metaX &&
           mono.getX() < metaX + metaW &&
           mono.getY() > metaY - 40 &&
           mono.getY() < metaY + metaH;
    }
    public void levelUp(Player p) {
        System.out.println("=== LEVEL UP ===");
        // Enviar mensaje WIN SOLO a ese jugador
        // 1) MANDAR WIN AL PLAYER
        ClientHandler handler = pantallas.get(p.getPantallaId());
        if (handler != null) {
            handler.send("WIN\n");
            System.out.println("[SERVER] WIN enviado a " + p.getPantallaId());
        }

        // Subir nivel lógico
        level++;
        p.nextLevel();

        // Aumentar velocidad global
        velocidadFactor *= 1.20f;

        // Aumentar la velocidad de TODOS los cocodrilos existentes
        for (GameObject obj : objetos) {
            obj.update(1.20f);
        }

        // Reiniciar mono
        p.getMono().setPosition(100, 700);

        
    }


    //======================================
    private void processInput(String msg, float dt) {
        try {
            // Formato: "INPUT <tick> <up> <down> <left> <right> <jump> <pantallaId>"
            String[] p = msg.split("\\s+");
            if (p.length < 8) return;
            int up = Integer.parseInt(p[2]);
            int down = Integer.parseInt(p[3]);
            int left = Integer.parseInt(p[4]);
            int right = Integer.parseInt(p[5]);
            int jump = Integer.parseInt(p[6]);
            String pantallaId = p[7];

            Player targetPlayer = null;
            for (Player player : players.values()) {
                if (pantallaId.equals(player.getPantallaId())) {
                    targetPlayer = player;
                    break;
                }
            }
            if (targetPlayer != null) {
            targetPlayer.getMono().applyInput(up, down, left, right, jump, dt);
        }
        } catch (Exception e) {
            System.err.println("Error procesando input: " + msg);
            e.printStackTrace();
        }
    }

    private void detectarcolisiones(){
        for (Player player : players.values()) {
            // Obtener personaje de juego y pantalla
            DKJr mono = player.getMono();
            String pantalla = player.getPantallaId();

            //Posicion del mono
            float mx = mono.getX();
            float my = mono.getY();

            //Detectar caída
            if (mono.hasFallen()) {
                player.killPlayer();
                System.out.println("[CAIDA] Jugador " + player.getPlayerId() + " murió por caída");
                mono.clearFallen();
                continue;
            }

            Iterator<GameObject> it = objetos.iterator();

            while (it.hasNext()) {
                // -------------------------
                //  COLISIÓN CON COCODRILO
                // -------------------------
                GameObject o = it.next();
                
                // Buscar para cada croc en x pantalla
                if (o instanceof BlueCroc) {
                    if (!pantalla.equals(o.getPantallaDisplay())) continue;

                    float dx = Math.abs(mx - o.getX());
                    float dy = Math.abs(my - o.getY());

                    if (dx < 40 && dy < 40) {
                         player.killPlayer();
                        System.out.println("[COLISION] Jugador " + player.getPlayerId() + " murió");
                        break;
                    }
                    continue;
                }

                // -------------------------
                //  COLISIÓN CON COCODRILO
                // -------------------------
                
                // Buscar para cada croc en x pantalla
                if (o instanceof RedCroc) {
                    if (!pantalla.equals(o.getPantallaDisplay())) continue;

                    float dx = Math.abs(mx - o.getX());
                    float dy = Math.abs(my - o.getY());

                    if (dx < 40 && dy < 40) {
                         player.killPlayer();
                        System.out.println("[COLISION] Jugador " + player.getPlayerId() + " murió");
                        break;
                    }
                    continue;
                }
                // -------------------------
                //  COLISIÓN CON FRUTA
                // -------------------------
                if (o.getClass().getSimpleName().equals("Fruit")) {
                    if (!pantalla.equals(o.getPantallaDisplay())) continue;

                    float dx = Math.abs(mx - o.getX());
                    float dy = Math.abs(my - o.getY());

                    if (dx < 40 && dy < 40) {

                        int pts = o.getPuntos();   // obtener puntos de la fruta
                        player.addScore(pts);      // sumar puntos al jugador

                        System.out.println("[FRUTA] Jugador " + player.getPlayerId() +
                            " obtuvo +" + pts + " puntos!");

                        it.remove();               // eliminar fruta del mapa
                    }
                }
            }
        }
    }

    public String toJson() {
        // JSON sencillo a mano (puedes cambiar a una lib luego)
        StringBuilder sb = new StringBuilder();
        sb.append("{\"tick\":").append(tick);
        sb.append(",\"players\":{");
        
        // Incluir TODOS los jugadores
        boolean first = true;
        for (Map.Entry<String, Player> entry : players.entrySet()) {
            if (!first) sb.append(",");
            sb.append("\"").append(entry.getValue().getPantallaId()).append("\":");
            sb.append(entry.getValue().toJson());
            first = false;
        }
        // Cerrar players y abrir objetos
        sb.append("},\"objetos\":[");
        first = true;
        // Enviar SOLO objetos de esta pantalla
        for (GameObject o : objetos) {
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
        
        sb.append(",\"plataformas\":[");
        for (int i = 0; i < plataformas.size(); i++) {
            if (i > 0) sb.append(",");
            sb.append(plataformas.get(i).toJson());
        }
        sb.append("]");

        //for (int i = 0; i < crocs.size(); i++) {
        //  if (i>0) sb.append(",");
        //  sb.append(crocs.get(i).toJson());
        //}
        //sb.append("],\"fruits\":[");
        //for (int i = 0; i < fruits.size(); i++) {
        //  if (i>0) sb.append(",");
        //  sb.append(fruits.get(i).toJson());
        //}
        sb.append("}");
        return sb.toString();
    }
}