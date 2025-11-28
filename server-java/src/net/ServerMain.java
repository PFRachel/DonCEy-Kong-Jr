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
 * *  Entradas:
 *      - Argumentos de línea de comando (opcional futuro).
 *
 *  Salidas:
 *      - Inicio del servidor en el puerto configurado.
 *
 *  Restricciones:
 *      - El puerto debe estar disponible para su uso.
 *      - No se permite iniciar múltiples servidores en el mismo
 *        proceso.
 *
 * ---------------------------------------------------------------
 */
package net;

public class ServerMain {
    public static void main(String[] args) {
        int port = 5050;
        GameServer server = new GameServer(port);
        server.start(); // inicia sockets + bucle de juego
    }
}
