/**
 * ---------------------------------------------------------------
 *  Nombre del archivo: ServerMain.java
 *  Paquete: net
 *  Descripción:
 *      Clase principal del servidor de juego.
 *      Inicia el servidor en el puerto especificado y lanza
 *      el bucle principal de conexión con los clientes.
 * 
 * ACÁ EJECUTAS EL CODIGO PARA INICIAR SERVER
 * ---------------------------------------------------------------
 */
package net;

import logic.GameState;   
import logic.GameLoop;

public class ServerMain {
    public static void main(String[] args) {
        int port = 5050;
        GameServer server = new GameServer(port);
        server.start(); // inicia sockets + bucle de juego
    }
}
