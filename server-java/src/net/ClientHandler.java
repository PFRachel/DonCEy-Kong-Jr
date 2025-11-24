/**
 * ---------------------------------------------------------------
 *  Nombre del archivo: ClientHandler.java
 *  Paquete: net
 *  Descripción:
 *      Manejador individual de un cliente conectado al servidor.
 *      Escucha mensajes del cliente, los procesa y actualiza
 *      el estado del juego según las acciones recibidas.
 * ---------------------------------------------------------------
 */
package net;

import logic.GameState;

import java.io.*;
import java.net.Socket;

public class ClientHandler implements Runnable {
    
    private final Socket socket;
    private final GameState state;
    private final GameServer server;
    private PrintWriter out;
    //-----------
    private String pantallaId;
    private String tipo;

    public void setPantallaId(String id){ this.pantallaId = id; }
    public String getPantallaId(){ return pantallaId; }

    public void setTipo(String t){ this.tipo = t; }
    public String getTipo(){ return tipo; }


    public ClientHandler(Socket socket, GameState state, GameServer server) {
        this.socket = socket; this.state = state; this.server = server;
    }

    public void send(String line) {
        if (out != null) { out.print(line); out.flush(); }
    }

    @Override public void run() {
        System.out.println("[Server] Client connected " + socket.getRemoteSocketAddress());

        try (
           BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        ) {
            out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())), true);
            String line;
            // ACA EL SERVER RECIBE MENSAJES DEL CLIENTE
            while ((line = in.readLine()) != null) {
                //System.out.println("[Server] Received: " + line);
                handle(line.trim());
            }
        } catch (IOException e) {
            System.out.println("[Server] Client IO closed: " + e.getMessage());
        } finally {
            // Avisar al GameState que este cliente se desconectó
            state.onClientDisconnected(this);
            server.remove(this);
            try { socket.close(); } catch (IOException ignored) {}
        }
    }

    private void handle(String msg) {
        if (msg.startsWith("JOIN_PLAYER")) {
            String nick = msg.substring("JOIN_PLAYER".length()).trim();
            if (state.addPlayer(nick, this)) {
                // IMPORTANTE: enviar qué pantalla le tocó a este jugador
                // para que el cliente C pueda poner miPantallaId = "pantalla1" o "pantalla2"
                String pantalla = getPantallaId(); // "pantalla1" o "pantalla2"
                if (pantalla != null) {
                    send("ACK " + pantalla + "\n");
                } else {
                    // Por seguridad, al menos mandar ACK simple
                    send("ACK\n");
                }
            } else {
                send("ERR max_players\n");
                try { socket.close(); } catch (IOException ignored) {}
            }
        } else if (msg.startsWith("JOIN_SPECTATOR")) {
            String nick = msg.substring("JOIN_SPECTATOR".length()).trim();
            if (state.addSpectator(nick, this)) {
                String target = state.getSpectatorTargetFor(this);
                if (target != null) {
                    send("ACK " + target + "\n");
                } else {
                    send("ACK\n");
                }
            } else {
                send("ERR max_spectators\n");
                try { socket.close(); } catch (IOException ignored) {}
            }
            // INPUT <tick> <up> <down> <left> <right> <jump>
        } else if (msg.startsWith("INPUT")) {
            String fullmsg = msg + " " + pantallaId;
            state.enqueueInput(fullmsg);
        } else if (msg.startsWith("ADMIN_SPAWN_CROC")) {
            state.adminSpawnCroc(msg);// validación de permisos pendiente
        } else if (msg.startsWith("ADMIN_SPAWN_FRUIT")) {
            state.adminSpawnFruit(msg);
        } else if (msg.startsWith("ADMIN_DELETE_FRUIT")) {
            state.adminDeleteFruit(msg);
        } else if (msg.startsWith("LEAVE")) {
            // opcional: limpieza
            // se maneja en state si quieres
        } else {
            send("ERR unknown\n");
        }
    }
}