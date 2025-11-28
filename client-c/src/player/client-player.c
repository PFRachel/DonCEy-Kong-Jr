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
// Entradas:
//      Datos del servidor (JSON, señales WIN/GAME_OVER),
//      inputs del jugador y texturas del juego.
//
// Salidas:
//      Mensajes TCP al servidor y renderizado gráfico local.
//
// Restricciones:
//      Solo Windows, requiere Winsock2 y Raylib inicializados.
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

    // win y gameover
    int mostrarWin = 0;
    int mostrarGameOver = 0;
    float winTimer = 0;

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
    char ackBuffer[128];
    int bytes = recv(serverSocket, ackBuffer, sizeof(ackBuffer)-1, 0);
    
    char miPantallaId[32] = "pantalla1"; // valor por default

    if (bytes > 0) {
        ackBuffer[bytes] = '\0';
        if (strstr(ackBuffer, "ACK") == NULL) {
            printf("Error: El servidor no aceptó la conexión.\n");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        char* pantalla_start = strstr(ackBuffer, "pantalla");
        if (pantalla_start) {

            // Extraer idpantalla
            sscanf(pantalla_start, "%s", miPantallaId);
            printf("[Player] PantallaId asignado: %s\n", miPantallaId);
        } else {
            printf("[Player] Usando por defecto: %s\n", 
                   miPantallaId);
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
    printf("Ventana inicializada: %dx%d\n", WIDTH, HEIGHT);

    // Cargar recursos
    cargarTexturas();

    // Inicializar estado del juego
    GameState miJuego;
    inicializarGameState(&miJuego);

    float deltaTime = 0.0f;

    int tick = 0;

    // Bucle Principal 
    while (!WindowShouldClose()) {
        deltaTime = obtenerDeltaTime();
        
        // Tomar el Input del User o detenerlo en gameover 
        int up, down, left, right, jump;

        if (mostrarGameOver) {
            up = down = left = right = jump = 0;   // sin input
        } else {
            up = IsKeyDown(KEY_UP);
            down = IsKeyDown(KEY_DOWN); 
            left = IsKeyDown(KEY_LEFT);
            right = IsKeyDown(KEY_RIGHT);
            jump = IsKeyDown(KEY_SPACE);
        }

 
        // Enviar al servidor los inputs
        char msg[128];
        snprintf(msg, sizeof(msg), "INPUT %d %d %d %d %d %d %s\n",
            tick, up, down, left, right, jump, miPantallaId);
        send(serverSocket, msg, strlen(msg), 0);
        tick++;

        // Recibir estado 
        char buffer[4096];
        int bytesRecibidos = recibirMensaje(serverSocket, buffer, sizeof(buffer)-1);
    
        // Procesar mensaje del server
        if (bytesRecibidos > 0) {
            // Asegurar terminación del string
            buffer[bytesRecibidos] = '\0';

            if (strstr(buffer, "WIN") != NULL) {
                mostrarWin = 1;
                winTimer = 1.5f;
                printf("=== WIN RECEIVED ===\n");
            }
            if (strstr(buffer, "GAME_OVER") != NULL) {
                mostrarGameOver = 1;
                printf("=== GAME OVER RECEIVED ===\n");
            }
             // Punto de inicio del JSON completo
            char* jsonStart = strstr(buffer, "{");
            if (!jsonStart) { 
                goto render_loop;//continue
            }
           
            if (strstr(buffer, "\"dkjr\":null") != NULL) {
                // No hay datos válidos, NO actualizar nada
                goto render_loop;
            }
            
            // Procesar objetos
            procesarMensajeObjetos(jsonStart, &miJuego, miPantallaId);

            // Buscar coordenadas en el JSON
            char key[64];
            sprintf(key, "\"%s\":", miPantallaId);  

            char* pantalla_obj = strstr(jsonStart, key);
            if (pantalla_obj) {
                char* id_pos = strstr(pantalla_obj, "\"id\":");
                char* score_pos = strstr(pantalla_obj, "\"score\":");
                char* lives_pos = strstr(pantalla_obj, "\"lives\":");
                char* level_pos = strstr(pantalla_obj, "\"level\":");
                if (id_pos) sscanf(id_pos, "\"id\":%d", &miJuego.player.id);
                if (score_pos) sscanf(score_pos, "\"score\":%d", &miJuego.player.score);
                if (lives_pos) sscanf(lives_pos, "\"lives\":%d", &miJuego.player.lives);
                if (level_pos) sscanf(level_pos, "\"level\":%d", &miJuego.player.level);
                // Buscar x y y SOLO dentro de su propio objeto
                char* x_pos = strstr(pantalla_obj, "\"x\":");
                char* y_pos = strstr(pantalla_obj, "\"y\":");

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
    
render_loop:
        // AUN NO ES FUNCIONAL
        actualizarAnimacionJugador(&miJuego.player);

        // Redenrizar grafico
        // DIBUJAR
        BeginDrawing();
        // Fondo
        
        
        // Renderizar juego 
        renderizarUI(&miJuego);

            // Labels
        DrawText("DONKEY KONG JR", 250, 50, 30, DARKBLUE);
        
        DrawText("JUGADOR:", 650, 40, 20, WHITE);
        DrawText(TextFormat("%s", miPantallaId), 750, 40, 20, WHITE);

        DrawText("PUNTOS:", 650, 60, 20, WHITE);
        DrawText(TextFormat("%d", miJuego.player.score), 750, 60, 20, YELLOW);

        DrawText("VIDAS:", 650, 80, 20, WHITE);
        DrawText(TextFormat("%d", miJuego.player.lives), 750, 80, 20, RED);

        DrawText("NIVEL:", 650, 100, 20, WHITE);
        DrawText(TextFormat("%d", miJuego.player.level), 750, 100, 20, GREEN);
        
        // >>> AGREGADO WIN <<<
        if (mostrarWin) {
            winTimer -= deltaTime;
            if (winTimer <= 0) {
                mostrarWin = 0;
            }
            DrawText("WIN!", 350, 200, 80, YELLOW);
        }

        if (mostrarGameOver) {
            DrawText("GAME OVER", 260, 350, 60, RED);
        }

        EndDrawing();
    }
        
    // Cerrar socket y limpiar Winsock
    descargarTexturas();
    CloseWindow();
    desconectarServidor(serverSocket);
    WSACleanup();
    
    return 0;
}