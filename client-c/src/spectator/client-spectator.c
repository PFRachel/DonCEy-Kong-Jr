// ---------------------------------------------------------------
// Nombre del archivo: client-spectator
// Descripción: 
//      Cliente TCP espectador para juegos LAN/Wi-Fi en Windows
//       - Se conecta a un servidor mediante sockets TCP
//       - Se registra como espectador
//       - Recibe estado del juego
// ---------------------------------------------------------------
// Solo hace JOIN_SPECTATOR y lee STATE para dibujar.
// ---------------------------------------------------------------

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
#include <stdlib.h>
#include <string.h>

#define PORT 5050
#define IP "192.168.100.41"   // <-- usa tu IP real
#define WIDTH 800
#define HEIGHT 600

SOCKET conectarServidor(const char* ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);

    // Convertir IP
    if (inet_pton(AF_INET, ip, &serv.sin_addr) <= 0) {
        printf("Error al convertir IP\n");
        return INVALID_SOCKET;
    }

    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) != 0) {
        printf("Error al conectar: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    return sock;
}

int main() {
    // Inicializar Winsock
    WSADATA w;
    if (WSAStartup(MAKEWORD(2,2), &w) != 0) {
        printf("Error Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET sock = conectarServidor(IP, PORT);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    printf("[Spectator] Conectado al servidor!\n");

    // Enviar solicitud de espectador
    char joinMsg[128];
    snprintf(joinMsg, sizeof(joinMsg), "JOIN_SPECTATOR %s\n", "Viewer1");
    send(sock, joinMsg, strlen(joinMsg), 0);

    // Recibir ACK
    char ack[64];
    int n = recv(sock, ack, sizeof(ack)-1, 0);
    if (n <= 0 || strstr(ack, "ACK") == NULL) {
        printf("Servidor rechazó conexión.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("[Spectator] Registro exitoso!\n");

    // Ventana Raylib
    InitWindow(WIDTH, HEIGHT, "DK Jr - Espectador");
    SetTargetFPS(60);

    char stateBuffer[4096] = "Esperando STATE...";

    while (!WindowShouldClose()) {

        // Recibir el estado del juego del servidor
        int bytes = recv(sock, stateBuffer, sizeof(stateBuffer)-1, 0);
        if (bytes > 0) {
            stateBuffer[bytes] = 0;
            // Opcionalmente limpiar "STATE " del inicio
            if (strncmp(stateBuffer, "STATE ", 6) == 0) {
                memmove(stateBuffer, stateBuffer+6, strlen(stateBuffer)-5);
            }
        }

        // Dibujar
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("ESPECTADOR", 20, 20, 20, RAYWHITE);
        DrawText("Estado recibido:", 20, 60, 20, GREEN);
        DrawText(stateBuffer, 20, 90, 16, YELLOW);

        EndDrawing();
    }

    CloseWindow();
    closesocket(sock);
    WSACleanup();
    return 0;
}
