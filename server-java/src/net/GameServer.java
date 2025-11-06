/**
 * ---------------------------------------------------------------
 *  Nombre del archivo: GameServer.java
 *  Paquete: net
 *  Descripción:
 *      Gestiona las conexiones de red con los clientes.
 *      Acepta múltiples clientes, crea hilos para cada uno
 *      y mantiene el estado global del juego mediante GameState.
 * ---------------------------------------------------------------
 */
package net;

import logic.GameLoop;
import logic.GameState;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Servidor de juego: acepta clientes (jugadores y espectadores)
 * y mantiene el estado global.
 */
public class GameServer {
    private final int port;
    private final CopyOnWriteArrayList<ClientHandler> clients = new CopyOnWriteArrayList<>();
    private final GameState state = new GameState();
    private GameLoop loop;

    public GameServer(int port) { this.port = port; }

    public void start() {
        loop = new GameLoop(state, this::broadcastState);
        loop.start(); // inicia el bucle a 60 Hz

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("[Server] Listening on " + port);
            System.out.println("[Server] Local IP: " + java.net.InetAddress.getLocalHost().getHostAddress());
            while (true) {
                Socket s = serverSocket.accept();
                ClientHandler h = new ClientHandler(s, state, this);
                clients.add(h);
                new Thread(h, "client-" + s.getPort()).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            loop.stop();
        }
    }

    public void remove(ClientHandler h) { clients.remove(h); }

    public void broadcast(String line) {
        for (ClientHandler c : clients) c.send(line);
    }

    private void broadcastState(String stateJson) {
        broadcast("STATE " + stateJson + "\n");
    }
}
