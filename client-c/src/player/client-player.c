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
    //if (registrarJugador(serverSocket, "Player1") != 0) {
       // printf("Error registrando jugador\n");
        //closesocket(serverSocket);
       // WSACleanup();
       // return 1;
    //}

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
        
        // Para saltos y detectar muertes por caida
        aplicarGravedad(&miJuego.player);

        // AUN NO ES FUNCIONAL
        actualizarAnimacionJugador(&miJuego.player);

        // PRUEBA PARA MOVER AL MONO, PERO DEBERIA DE ESTAR EN JAVA
        if (IsKeyDown(KEY_RIGHT)) miJuego.player.position.x += 200 * deltaTime;
        if (IsKeyDown(KEY_LEFT))  miJuego.player.position.x -= 200 * deltaTime;
        if (IsKeyDown(KEY_UP))    miJuego.player.position.y -= 200 * deltaTime;
        if (IsKeyDown(KEY_DOWN))  miJuego.player.position.y += 200 * deltaTime;

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