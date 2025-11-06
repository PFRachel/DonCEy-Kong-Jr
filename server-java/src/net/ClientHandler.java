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
            out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())), false);
            String line;
            while ((line = in.readLine()) != null) {
                handle(line.trim());
            }
        } catch (IOException e) {
            System.out.println("[Server] Client IO closed: " + e.getMessage());
        } finally {
            server.remove(this);
            try { socket.close(); } catch (IOException ignored) {}
        }
    }

    private void handle(String msg) {
        if (msg.startsWith("JOIN_PLAYER")) {
            String nick = msg.substring("JOIN_PLAYER".length()).trim();
            state.addPlayer(nick, this);
            send("ACK\n");
        } else if (msg.startsWith("JOIN_SPECTATOR")) {
            String nick = msg.substring("JOIN_SPECTATOR".length()).trim();
            state.addSpectator(nick, this);
            send("ACK\n");
        } else if (msg.startsWith("INPUT")) {
            // INPUT <tick> <up> <down> <left> <right> <jump>
            state.enqueueInput(msg);
        } else if (msg.startsWith("ADMIN_SPAWN_CROC")) {
            state.adminSpawnCroc(msg); // validación de permisos pendiente
        } else if (msg.startsWith("ADMIN_SPAWN_FRUIT")) {
            state.adminSpawnFruit(msg);
        } else if (msg.startsWith("ADMIN_DELETE_FRUIT")) {
            state.adminDeleteFruit(msg);
        } else if (msg.startsWith("LEAVE")) {
            // se maneja en state si quieres
        } else {
            send("ERR unknown\n");
        }
    }
}
