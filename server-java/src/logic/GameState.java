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

import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;

public class GameState {
    // listas «conceptuales»
    private final List<Croc> crocs = new ArrayList<>();
    private final List<Fruit> fruits = new ArrayList<>();
    private final Map<String, Player> players = new LinkedHashMap<>();
    private final ConcurrentLinkedQueue<String> inputQueue = new ConcurrentLinkedQueue<>();
    private final DKJr dkjr = new DKJr();

    private float velocidadFactor = 1.0f;
    private long tick = 0;

    // ---- API desde la red ----
    public void addPlayer(String nick, Object conn) {
        if (players.size() >= 2) return; // regla de negocio
        players.put(nick, new Player(nick));
    }
    public void addSpectator(String nick, Object conn) {/* registrar si deseas*/}

    public void enqueueInput(String line) { inputQueue.add(line); }

    public void adminSpawnCroc(String line) {
        // "ADMIN_SPAWN_CROC Azul 3 1.5"
        String[] p = line.split("\\s+");
        String tipo = p[1]; int liana = Integer.parseInt(p[2]); float vel = Float.parseFloat(p[3]);
        crocs.add(new Croc(tipo, liana, vel));
    }
    public void adminSpawnFruit(String line) {
        String[] p = line.split("\\s+");
        int liana = Integer.parseInt(p[1]); float altura = Float.parseFloat(p[2]); int puntos = Integer.parseInt(p[3]);
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
            // INPUT <tick> <up> <down> <left> <right> <jump>
            String[] p = msg.split("\\s+");
            int up = Integer.parseInt(p[2]);
            int down = Integer.parseInt(p[3]);
            int left = Integer.parseInt(p[4]);
            int right = Integer.parseInt(p[5]);
            int jump = Integer.parseInt(p[6]);
            dkjr.applyInput(up, down, left, right, jump);
        }

        // 2) mover cocodrilos
        for (Croc c : crocs) c.update(dt * velocidadFactor);

        // 3) colisiones (simplificado)
        // ... detectar y actualizar vidas/puntaje

        // 4) reglas de rescate → subir dificultad
        // if (rescato) { dkjr.vidas++; velocidadFactor *= 1.10f; reiniciarNivel(); }
    }

    public String toJson() {
        // JSON sencillo a mano (puedes cambiar a una lib luego)
        StringBuilder sb = new StringBuilder();
        sb.append("{\"tick\":").append(tick)
          .append(",\"dkjr\":").append(dkjr.toJson())
          .append(",\"crocodiles\":[");
        for (int i = 0; i < crocs.size(); i++) {
            if (i>0) sb.append(",");
            sb.append(crocs.get(i).toJson());
        }
        sb.append("],\"fruits\":[");
        for (int i = 0; i < fruits.size(); i++) {
            if (i>0) sb.append(",");
            sb.append(fruits.get(i).toJson());
        }
        sb.append("]}");
        return sb.toString();
    }

    // ---- modelos «POJO» mínimos ----
    static class Player { String nick; Player(String n){nick=n;} }
    static class DKJr {
        float x=0, y=0; int vidas=3, puntaje=0;
        void applyInput(int up,int down,int left,int right,int jump){
            float v=2.0f;
            if(up==1) y+=v; if(down==1) y-=v; if(left==1) x-=v; if(right==1) x+=v;
            if(jump==1) {/* lógicas de salto */}
        }
        String toJson(){ return "{\"x\":"+x+",\"y\":"+y+",\"vidas\":"+vidas+",\"puntaje\":"+puntaje+"}"; }
    }
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
