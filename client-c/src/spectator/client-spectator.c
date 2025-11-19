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
    int   hasPlayer;   // 1 = hay jugador conectado, 0 = no hay jugador
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
// Ahora recibe también qué pantalla queremos observar ("pantalla1", "pantalla2", ...)
void updateRenderFromJson(RenderContext* rc, const char* buffer, const char* pantallaIdDestino) {
    // Por defecto, asumimos que NO hay jugador para esa pantalla
    rc->hasPlayer = 0;

    const char* json = strstr(buffer, "{");
    if (!json) return;

    // Construir clave de la pantalla: "pantalla1":  o  "pantalla2":
    char key[64];
    snprintf(key, sizeof(key), "\"%s\":", pantallaIdDestino);

    const char* pantallaObj = strstr(json, key);
    if (!pantallaObj) {
        // No se encontró esa pantalla en este STATE → no hay jugador conectado
        return;
    }

    // Ir al inicio del objeto del jugador
    const char* objStart = strchr(pantallaObj, '{');
    if (!objStart) return;

    // Buscar las coordenadas dentro de ese objeto
    const char* xPtr = strstr(objStart, "\"x\":");
    const char* yPtr = strstr(objStart, "\"y\":");

    if (xPtr) rc->dkjrX = (float)atof(xPtr + 4);
    if (yPtr) rc->dkjrY = (float)atof(yPtr + 4);

    // Si llegamos aquí, sí hay datos de jugador
    rc->hasPlayer = 1;
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
    RenderContext rc = {200, 300, 0};

    // Pantalla (jugador) que se quiere observar inicialmente
    // "pantalla1" -> Jugador 1, "pantalla2" -> Jugador 2
    char pantallaObjetivo[32] = "pantalla1";

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

        // Cambiar de jugador observado con teclas 1 y 2
        if (IsKeyPressed(KEY_ONE)) {
            strcpy(pantallaObjetivo, "pantalla1");
        }
        if (IsKeyPressed(KEY_TWO)) {
            strcpy(pantallaObjetivo, "pantalla2");
        }

        int n = recvNoBlock(sock, buffer, sizeof(buffer));
        if (n < 0) break;
        if (n > 0 && strstr(buffer, "STATE")) {
            // ← OBSERVER aquí, ahora filtrando por pantallaObjetivo
            updateRenderFromJson(&rc, buffer, pantallaObjetivo);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(fondoTexture, 0, 0, WHITE);

        DrawText("ESPECTADOR (Observer Remoto)", 20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Observando: %s (teclas 1 y 2)", pantallaObjetivo),
                 20, 50, 18, RAYWHITE);

        if (rc.hasPlayer) {
            // Hay jugador para esa pantalla → dibujar mono
            DrawTexture(playerTexture, (int)rc.dkjrX, (int)rc.dkjrY, WHITE);
        } else {
            // No hay jugador conectado para esa pantalla
            const char* msg = "NO HAY JUGADOR CONECTADO";
            int fontSize = 30;
            int textWidth = MeasureText(msg, fontSize);
            DrawText(msg,(WIDTH - textWidth)/2, HEIGHT/2, fontSize, RED);
        }

        EndDrawing();
    }

    CloseWindow();
    closesocket(sock);
    WSACleanup();
    return 0;
}