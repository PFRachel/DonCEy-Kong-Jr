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

#ifdef _WIN32
//  Evitar conflictos entre Raylib y Windows
#define WIN32_LEAN_AND_MEAN
#define NOGDI       
#define NOUSER     
#define NOMINMAX 

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#error Solo windows
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include "../../lib/raylib/include/raylib.h"

// IP y PUERTO para modificar
#define PORT 5050
#define IP "192.168.124.8" // localhost
#define WIDTH 800
#define HEIGHT 600

// Estructura del jugador
typedef struct {
    Vector2 position; //Posicion Inicial
    int width, height; //Dimensiones del Objeto
    Color color; 
    char name[32];
    int score;
    bool connected; // Control del estado
} Player;

int main() {
    // Inicializar Winsock
    int res;
    WSADATA w; 
    res = WSAStartup(MAKEWORD(2,2), &w);
    if (res){
        printf("Error: %d\n", res);
        return 1;
    }

    // Crear socket cliente
    SOCKET client;
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET){
        printf("Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configurar direccion de IP
    struct sockaddr_in  serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &serv.sin_addr);

    // Conectar al servidor
    bool conectado = false;
    if (connect(client, (struct sockaddr*)&serv, sizeof(serv)) == 0) {
        conectado = true;
        printf("[Player] Conectado al servidor %s:%d\n", IP, PORT);
        
        // Registrarse como jugador
        char buf[2048];
        snprintf(buf, sizeof(buf), "JOIN_PLAYER Player1\n");
        send(client, buf, (int)strlen(buf), 0);
    } else {
        printf("Error al conectar: %d\n", WSAGetLastError());
    }

// ============================================================================
    // 1. Iniciar Ventana
    InitWindow(WIDTH, HEIGHT, "Donkey Kong Jr - Player");
    SetTargetFPS(60);
    
    // 2. (Temporal) Configurar usuario
    char username[] = "Player1";

    int tick = 0; // tiempo actualizacion de estados

    // 3. Bucle Principal (interfaz + socket)
    while (!WindowShouldClose()) {
        // Input del User
        int up = IsKeyDown(KEY_UP);
        int down = IsKeyDown(KEY_DOWN); 
        int left = IsKeyDown(KEY_LEFT);
        int right = IsKeyDown(KEY_RIGHT);
        int jump = IsKeyPressed(KEY_SPACE);
        
        // Socket
        if (conectado) {
            char buf[256];
            
            // Enviar inputs al servidor
            snprintf(buf, sizeof(buf), "INPUT %d %d %d %d %d\n",
                     up, down, left, right, jump);
            send(client, buf, (int)strlen(buf), 0);

            // Recibir datos del servidor 
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(client, &readfds);
            
            struct timeval timeout = {0, 1000}; // 1ms timeout
            
            if (select(0, &readfds, NULL, NULL, &timeout) > 0) {
                int n = recv(client, buf, sizeof(buf)-1, 0);
                if (n > 0) { 
                    buf[n] = 0; 
                    printf("Servidor: %s", buf); 
                }
            }
            tick++;
        }  

        // Redenrizar grafico
        // DIBUJAR
        BeginDrawing();
            // Fondo
            ClearBackground(SKYBLUE);
            
            // Plataforma 
            DrawRectangle(0, 500, WIDTH, 100, BROWN);
            
            // Jugador temporal
            DrawRectangle(100, 400, 40, 60, RED);
            
             // Labels
            DrawText("DONKEY KONG JR", 250, 50, 30, DARKBLUE);
            DrawText("JUGADOR:", 50, 120, 25, WHITE);
            DrawText(username, 200, 120, 25, YELLOW);
            DrawText("Presiona ESC para salir", 250, 550, 15, LIGHTGRAY);
        EndDrawing();
    }

    // Cerrar socket y limpiar Winsock
    CloseWindow();
    if (conectado) {
        closesocket(client);
    }
    WSACleanup();
    return 0;
}
