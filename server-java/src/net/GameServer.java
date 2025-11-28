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

import logic.GameState;
import logic.factory.CrocFactory;
import logic.factory.FruitFactory;
import logic.observer.GameStateObserver;
import logic.observer.ObservableGameLoop;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.concurrent.CopyOnWriteArrayList;

import domain.entities.Fruit;

/**
 * Servidor de juego: acepta clientes (jugadores y espectadores)
 * y mantiene el estado global.
 */
public class GameServer {
    private final int port;
    private final CopyOnWriteArrayList<ClientHandler> clients = new CopyOnWriteArrayList<>();
    private final GameState state = new GameState();

    private ObservableGameLoop observableLoop;

    public GameServer(int port) { this.port = port; }

    public void start() {
        // HILO PARA LEER COMANDOS DE CONSOLA (ADMIN)
        new Thread(() -> {
            Scanner sc = new Scanner(System.in);
            System.out.println("[ADMIN] Comandos activos: ADMIN_SPAWN_CROC y ADMIN_SPAWN_FRUIT");
            System.out.println("  ADMIN_SPAWN_CROC <color> <lianaIndex> <pantalla>");
            System.out.println("    Ej: ADMIN_SPAWN_CROC blue 3 pantalla1");
            System.out.println("  ADMIN_SPAWN_FRUIT <lianaIndex> <altura> <puntos> <pantalla>");
            System.out.println("    Ej: ADMIN_SPAWN_FRUIT 2 90 40 pantalla1");
            System.out.println("  ADMIN_DELETE_FRUIT <lianaIndex> <altura> <pantalla>");
            System.out.println("    Ej: ADMIN_DELETE_FRUIT 2 90 pantalla1");

            while (true) {
                String line = sc.nextLine().trim();
                procesarComandoAdmin(line);
            }
        }, "AdminConsole").start();

        // ---------------------------------------------------------------
        // USO DEL PATRÓN OBSERVER (JAVA → SUBJECT)
        // ---------------------------------------------------------------

        // Crear el Subject que envuelve el GameLoop lógico del juego
        observableLoop = new ObservableGameLoop(state);

        // Registrar un Observer que envía el estado JSON a TODOS los clientes
        observableLoop.addObserver(new GameStateObserver() {
            @Override
            public void onStateJson(String stateJson) {
                // Mismo formato que antes para el spectator y player
                broadcast("STATE " + stateJson + "\n");
            }
        });

        // Iniciar el bucle de juego (60 Hz)
        observableLoop.start();

        // ---------------------------------------------------------------
        // SERVIDOR TCP 
        // ---------------------------------------------------------------

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
            if (observableLoop != null) {
                observableLoop.stop();
            }
        }
    }

    public void remove(ClientHandler h) {
        clients.remove(h);
    }

    public void broadcast(String line) {
        for (ClientHandler c : clients) c.send(line);
    }
     // ======================================
    // MANEJO DE COMANDOS ADMIN
    // ======================================
    private void procesarComandoAdmin(String cmd) {

        String[] p = cmd.split("\\s+");
        if (p.length == 0) return;

        switch (p[0]) {

            // ---------------------------------------------
            case "ADMIN_SPAWN_CROC":
                if (p.length != 4) {
                    System.out.println("Uso: ADMIN_SPAWN_CROC <color> <liana> <pantalla>");
                    return;
                }

                String color = p[1];
                String lianaStr = p[2];
                String pantalla = p[3];

                // validar número
                try {
                    Integer.parseInt(lianaStr);
                } catch (NumberFormatException ex) {
                    System.out.println("[ADMIN] liana debe ser un entero.");
                    return;
                }

                String msgCroc = String.format("ADMIN_SPAWN_CROC %s %s %s", color, lianaStr, pantalla);
                state.adminSpawnCroc(msgCroc);

                broadcast("STATE " + state.toJson() + "\n");
                System.out.println("[ADMIN] Ejecutado: " + msgCroc);
                break;

            // ---------------------------------------------
            case "ADMIN_SPAWN_FRUIT":
                if (p.length != 5) {
                    System.out.println("Uso: ADMIN_SPAWN_FRUIT <liana> <altura> <puntos> <pantalla>");
                    return;
                }

                String lStr = p[1];
                String altura = p[2];
                String puntos = p[3];
                String pant = p[4];

                try {
                    Integer.parseInt(lStr);
                    Float.parseFloat(altura);
                    Integer.parseInt(puntos);
                } catch (NumberFormatException ex) {
                    System.out.println("[ADMIN] lianaIndex, altura o puntos inválidos.");
                    return;
                }

                String msgFruit = String.format("ADMIN_SPAWN_FRUIT %s %s %s %s", lStr, altura, puntos, pant);
                state.adminSpawnFruit(msgFruit);

                broadcast("STATE " + state.toJson() + "\n");
                System.out.println("[ADMIN] Ejecutado: " + msgFruit);
                break;

            // ---------------------------------------------
            case "ADMIN_DELETE_FRUIT":
                if (p.length != 4) {
                    System.out.println("Uso: ADMIN_DELETE_FRUIT <liana> <altura> <pantalla>");
                    return;
                }

                try {
                    Integer.parseInt(p[1]);
                    Float.parseFloat(p[2]);
                } catch (NumberFormatException ex) {
                    System.out.println("[ADMIN] formato inválido.");
                    return;
                }

                String msgDel = String.format("ADMIN_DELETE_FRUIT %s %s %s", p[1], p[2], p[3]);
                state.adminDeleteFruit(msgDel);

                broadcast("STATE " + state.toJson() + "\n");
                System.out.println("[ADMIN] Ejecutado: " + msgDel);
                break;

            default:
                System.out.println("[ADMIN] Comando no reconocido.");
        }
    }
    
}