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
                try {
                    if (line.equalsIgnoreCase("EXIT")) {
                        System.out.println("[ADMIN] Saliendo servidor (no recomendado en prod).");
                        System.exit(0);
                    } else if (line.startsWith("ADMIN_SPAWN_CROC")) {
                        procesarSpawnCroc(line);
                    } else if (line.startsWith("ADMIN_SPAWN_FRUIT")) {
                        procesarSpawnFruit(line);
                    } else if (line.startsWith("ADMIN_DELETE_FRUIT")) {
                        procesarDeleteFruit(line);
                    } else {
                        System.out.println("[ADMIN] Comando no reconocido: " + line);
                    }
                } catch (Exception e) {
                    System.out.println("[ADMIN] Error procesando comando: " + e.getMessage());
                    e.printStackTrace(System.out);
                }
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
     // --------- Métodos auxiliares para procesar comandos admin ----------
    private void procesarSpawnCroc(String line) {
        // Formato esperado: ADMIN_SPAWN_CROC <color> <lianaIndex> <pantalla>
        String[] p = line.split("\\s+");
        if (p.length != 4) {
            System.out.println("[ADMIN] Uso: ADMIN_SPAWN_CROC <color> <lianaIndex> <pantalla>");
            return;
        }
        String color = p[1];
        String lianaStr = p[2];
        String pantalla = p[3];

        int lianaIdx;
        try {
            lianaIdx = Integer.parseInt(lianaStr);
        } catch (NumberFormatException ex) {
            System.out.println("[ADMIN] lianaIndex debe ser un entero. Recibido: " + lianaStr);
            return;
        }

        // Construimos el mensaje tal como lo espera GameState.adminSpawnCroc
        String msg = String.format("ADMIN_SPAWN_CROC %s %d %s", color, lianaIdx, pantalla);
        state.adminSpawnCroc(msg);
        // Notificar a clientes inmediatamente
        broadcast("STATE " + state.toJson() + "\n");
        System.out.println("[ADMIN] Ejecutado: " + msg);
    }

    private void procesarSpawnFruit(String line) {
        // Formato esperado: ADMIN_SPAWN_FRUIT <lianaIndex> <altura> <puntos> <pantalla>
        String[] p = line.split("\\s+");
        if (p.length != 5) {
            System.out.println("[ADMIN] Uso: ADMIN_SPAWN_FRUIT <lianaIndex> <altura> <puntos> <pantalla>");
            return;
        }
        String lianaStr = p[1];
        String alturaStr = p[2];
        String puntosStr = p[3];
        String pantalla = p[4];

        try {
            Integer.parseInt(lianaStr);
            Float.parseFloat(alturaStr);
            Integer.parseInt(puntosStr);
        } catch (NumberFormatException ex) {
            System.out.println("[ADMIN] lianaIndex/altura/puntos con formato inválido.");
            return;
        }

        String msg = String.format("ADMIN_SPAWN_FRUIT %s %s %s %s", lianaStr, alturaStr, puntosStr, pantalla);
        state.adminSpawnFruit(msg);
        broadcast("STATE " + state.toJson() + "\n");
        System.out.println("[ADMIN] Ejecutado: " + msg);
    }

    private void procesarDeleteFruit(String line) {
        // Formato esperado: ADMIN_DELETE_FRUIT <lianaIndex> <altura> <pantalla>
        String[] p = line.split("\\s+");
        if (p.length != 4) {
            System.out.println("[ADMIN] Uso: ADMIN_DELETE_FRUIT <lianaIndex> <altura> <pantalla>");
            return;
        }
        String lianaStr = p[1];
        String alturaStr = p[2];
        String pantalla = p[3];

        try {
            Integer.parseInt(lianaStr);
            Float.parseFloat(alturaStr);
        } catch (NumberFormatException ex) {
            System.out.println("[ADMIN] lianaIndex/altura con formato inválido.");
            return;
        }

        String msg = String.format("ADMIN_DELETE_FRUIT %s %s %s", lianaStr, alturaStr, pantalla);
        state.adminDeleteFruit(msg);
        broadcast("STATE " + state.toJson() + "\n");
        System.out.println("[ADMIN] Ejecutado: " + msg);
    }
}