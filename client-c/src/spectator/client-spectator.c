// ---------------------------------------------------------------
// Nombre del archivo: client-spectator.c
// Descripción:
//   Cliente TCP ESPECTADOR con Raylib usando patrón OBSERVER.
//   - NetworkSubject recibe el estado del servidor (STATE {...})
//   - Observers reaccionan cuando hay nuevo GameStateC
//   - Este cliente SOLO observa, no envía INPUTs
//   - Observador REMOTO del Subject Java (ObservableGameLoop)
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

// ----- RenderContext: datos que se actualizan cuando llega STATE -----
typedef struct RenderContext {
    float dkjrX;
    float dkjrY;
} RenderContext;

// ----- Obtener socket conectado -----
SOCKET conectarServidor(const char* ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }
    return sock;
}

// ----- Recibir sin bloquear -----
int recvNoBlock(SOCKET sock, char* buf, int size) {
    fd_set set;
    struct timeval timeout = {0, 1000}; // 1 ms

    FD_ZERO(&set);
    FD_SET(sock, &set);
    if (select(0, &set, NULL, NULL, &timeout) <= 0)
        return 0;

    int n = recv(sock, buf, size - 1, 0);
    if (n > 0) buf[n] = '\0';
    return n;
}

// ----- OBSERVER REMOTO: reacciona al JSON -----
void updateRenderFromJson(RenderContext* rc, const char* buffer) {
    const char* json = strstr(buffer, "{");
    if (!json) return;

    const char* dk = strstr(json, "\"dkjr\"");
    if (!dk) return;

    const char* xPtr = strstr(dk, "\"x\":");
    const char* yPtr = strstr(dk, "\"y\":");

    if (xPtr) rc->dkjrX = (float)atof(xPtr + 4);
    if (yPtr) rc->dkjrY = (float)atof(yPtr + 4);
}

// ---------------------------------------------------------------
// MAIN — OBSERVER REMOTO
// ---------------------------------------------------------------
int main() {
    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);

    SOCKET sock = conectarServidor(IP, PORT);
    if (sock == INVALID_SOCKET) {
        printf("Error conectando a servidor.\n");
        return 1;
    }

    // Registrarse como spectator:
    send(sock, "JOIN_SPECTATOR Spectator\n", 26, 0);
    char ack[64];
    recv(sock, ack, sizeof(ack)-1, 0);

    // Contexto que se actualizará como OBSERVER
    RenderContext rc = {200, 300};

    InitWindow(WIDTH, HEIGHT, "Spectator - DonCEy Kong Jr");
    SetTargetFPS(60);

    Image fondoImg = LoadImage("client-c/image/Fondo.png");
    Texture2D fondoTexture = LoadTextureFromImage(fondoImg);
    UnloadImage(fondoImg);

    Image playerImg = LoadImage("client-c/image/IPlayer.png");
    Texture2D playerTexture = LoadTextureFromImage(playerImg);
    UnloadImage(playerImg);

    char buffer[2048];

    while (!WindowShouldClose()) {
        int n = recvNoBlock(sock, buffer, sizeof(buffer));
        if (n < 0) break;
        if (n > 0 && strstr(buffer, "STATE")) {
            updateRenderFromJson(&rc, buffer); // ← OBSERVER aquí
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(fondoTexture, 0, 0, WHITE);
        DrawTexture(playerTexture, (int)rc.dkjrX, (int)rc.dkjrY, WHITE);
        DrawText("ESPECTADOR (Observer Remoto)", 20, 20, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    closesocket(sock);
    WSACleanup();
    return 0;
}