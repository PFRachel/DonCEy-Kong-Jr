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
    char joinMsg[64];
    snprintf(joinMsg, sizeof(joinMsg), "JOIN_PLAYER Player1\n");
    send(serverSocket, joinMsg, strlen(joinMsg), 0);

    // Esperar confirmación
    char ackBuffer[64];
    int ackBytes = recibirMensaje(serverSocket, ackBuffer, sizeof(ackBuffer)-1);
    if (ackBytes > 0) {
        printf("[Player] Registrado en servidor: %s\n", ackBuffer);
    }

// ============================================================================
    // Iniciar Ventana
    InitWindow(WIDTH, HEIGHT, "Donkey Kong Jr - Player");
    SetTargetFPS(60);

    // Cargar recursos
    cargarTexturas();

    // Inicializar estado del juego
    GameState miJuego;
    inicializarGameState(&miJuego);

    char ultimoMensaje[256] = " Essperando mensaje del servidor...";
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
        int jump = IsKeyDown(KEY_SPACE);
 
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
            if (strstr(buffer, "\"dkjr\":null") != NULL) {
                // No hay datos válidos, NO actualizar nada
                continue;
            }
            // Buscar coordenadas en el JSON
            char* jsonStart = strstr(buffer, "{");
            if (jsonStart) {

                // Parsear X
                char* x_pos = strstr(jsonStart, "\"x\":");
                char* y_pos = strstr(jsonStart, "\"y\":");

                if (x_pos && y_pos) {
                    float x, y;
                    sscanf(x_pos, "\"x\":%f", &x);
                    sscanf(y_pos, "\"y\":%f", &y);

                    miJuego.player.position.x = x;
                    miJuego.player.position.y = y;
                }

                // Parsear estado de liana
                char* liana_pos = strstr(jsonStart, "\"onLiana\":");
                if (liana_pos) {
                    miJuego.player.onLiana = strstr(liana_pos, "true") != NULL;
                }

                // Parsear salto
                char* jump_pos = strstr(jsonStart, "\"jumping\":");
                if (jump_pos) {
                    miJuego.player.isJumping = strstr(jump_pos, "true") != NULL;
                }
            }
        }

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