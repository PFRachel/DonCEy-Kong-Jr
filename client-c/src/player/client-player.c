// ---------------------------------------------------------------
// Nombre del archivo: client-player
// Paquete: player
// Descripción: 
//      Cliente TCP para juegos LAN/Wi-Fi en Windows
//       - Se conecta a un servidor mediante sockets TCP
//       - Se registra como jugador
//       - Envía inputs 
//
// ACÁ EJECUTAS EL CODIGO PARA CREAR UN PLAYER
// ---------------------------------------------------------------

//  Evitar conflictos entre Raylib y Windows
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER  
#define NOMINMAX

#include "../../lib/raylib/include/raylib.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
//#pragma comment(lib, "ws2_32.lib")
#else
#error Solo windows
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include "../../headers/player.h"  
#include "../../headers/constants.h"  

int main() {
    // Inicializar Winsock
    WSADATA w; 
    if (WSAStartup(MAKEWORD(2,2), &w)!= 0){
        printf("Error: %d\n", WSAGetLastError());
        return 1;
    }

    // Conectar al servidor
    SOCKET serverSocket = conectarServidor(IP, PORT);
    if (serverSocket == INVALID_SOCKET){
        printf("Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Registrar jugador
    printf("[Player] Conectado al servidor %s:%d\n", IP, PORT);
    if (registrarJugador(serverSocket, "Player") != 0) {
       printf("Error registrando jugador\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    char ackBuffer[64];
    int bytes = recv(serverSocket, ackBuffer, sizeof(ackBuffer)-1, 0);
    if (bytes > 0) {
        ackBuffer[bytes] = '\0';
        if (strstr(ackBuffer, "ACK") == NULL) {
            printf("Error: El servidor no aceptó la conexión.\n");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
    } else {
        printf("Conexión cerrada por el servidor.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    printf("[Player] Registro exitoso. Entrando al juego...\n");

// ============================================================================
    // Iniciar Ventana
    InitWindow(WIDTH, HEIGHT, "Donkey Kong Jr - Player");
    SetTargetFPS(60);

    // Cargar recursos
    cargarTexturas();

    // Inicializar estado del juego
    GameState miJuego;
    inicializarGameState(&miJuego);

    char ultimoMensaje[256] = " Esperando mensaje del servidor...";
    float deltaTime = 0.0f;

    int tick = 0;

    // Bucle Principal 
    while (!WindowShouldClose()) {
        deltaTime = obtenerDeltaTime();
        
        // Tomar el Input del User
        int up = IsKeyDown(KEY_UP);
        int down = IsKeyDown(KEY_DOWN); 
        int left = IsKeyDown(KEY_LEFT);
        int right = IsKeyDown(KEY_RIGHT);
        int jump = IsKeyPressed(KEY_SPACE);
 
        // Enviar al servidor los inputs
        char msg[64];
        snprintf(msg, sizeof(msg), "INPUT %d %d %d %d %d %d\n",
            tick, up, down, left, right, jump);
        send(serverSocket, msg, strlen(msg), 0);
        tick++;

        // Recibir estado 
        char buffer[2048];
        int bytesRecibidos = recibirMensaje(serverSocket, buffer, sizeof(buffer)-1);
        
        // Procesar mensaje del server
        if (bytesRecibidos > 0) {
            printf("[Cliente] JSON recibido: %s\n", buffer);
            
            // Buscar coordenadas en el JSON
            char* x_pos = strstr(buffer, "\"x\":");
            char* y_pos = strstr(buffer, "\"y\":");
            
            // Obtener coords de mensaje y mostrar a la pantalla
            if (x_pos && y_pos) {
                float x, y;
                if (sscanf(x_pos, "\"x\":%f", &x) == 1 && 
                    sscanf(y_pos, "\"y\":%f", &y) == 1) {
                    miJuego.player.position.x = x;
                    miJuego.player.position.y = y;
                }
            }
        }

        // Para saltos y detectar muertes por caida
        //aplicarGravedad(&miJuego.player);

        // AUN NO ES FUNCIONAL
        actualizarAnimacionJugador(&miJuego.player);

        // Redenrizar grafico
        // DIBUJAR
        BeginDrawing();
        // Fondo
        DrawTexture(fondoTexture, 0, 0, WHITE);
        
        // Renderizar juego 
        renderizarJuego(&miJuego);

            // Labels
        DrawText("DONKEY KONG JR", 250, 50, 30, DARKBLUE);
        DrawText("JUGADOR:", 250, 120, 25, WHITE);
        DrawText(TextFormat("%d", miJuego.player.score), 50, 180, 20, YELLOW);
        EndDrawing();
    }
        
    // Cerrar socket y limpiar Winsock
    descargarTexturas();
    CloseWindow();
    desconectarServidor(serverSocket);
    WSACleanup();
    
    return 0;
}