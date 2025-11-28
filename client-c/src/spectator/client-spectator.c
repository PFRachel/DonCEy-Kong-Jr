// ---------------------------------------------------------------
// Nombre del archivo: client-spectator.c
// Descripción:
//   Cliente TCP ESPECTADOR con Raylib.
//   - NO envía INPUTs
//   - Solo recibe "STATE {...}" del servidor Java
//   - Dibuja al jugador, cocodrilos y frutas de una pantalla
//   - Usa un MENÚ INICIAL para elegir si observar a Player 1 o Player 2
// ---------------------------------------------------------------
//   Cliente espectador TCP.
//   Entrada: recibe "STATE {...}".
//   Salida: render de jugador, frutas y cocodrilos.
//   Restricción: no envía inputs; solo observa una pantalla.

//  Evitar conflictos entre Raylib y Windows
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX

#include "../../lib/raylib/include/raylib.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#error Solo windows
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>

#include "../../headers/constants.h"   // WIDTH, HEIGHT, IP, PORT, MAX_COCODRILOS, MAX_FRUTAS

// ----- Estructuras simples para render -----

typedef struct {
    float x;
    float y;
    int   tipo;     // 0 = rojo, 1 = azul
    int   active;
} Cocodrilo;

typedef struct {
    float x;
    float y;
    int   active;
} Fruta;

typedef struct RenderContext {
    float dkjrX;
    float dkjrY;
    int   hasPlayer;        // 1 = hay jugador conectado

    char  pantallaObjetivo[32]; // "pantalla1" o "pantalla2"

    Cocodrilo cocodrilos[MAX_COCODRILOS];
    Fruta     frutas[MAX_FRUTAS];

    int lives;
    int level;

    float winTimer; // tiempo restante para mostrar WIN
} RenderContext;

// ----- Conexión TCP -----

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

// Recibir sin bloquear
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

// ---------------------------------------------------------------
// Parseo de JSON muy simple (string search)
// ---------------------------------------------------------------

static void limpiarContextoObjetos(RenderContext* rc) {
    for (int i = 0; i < MAX_COCODRILOS; i++) {
        rc->cocodrilos[i].active = 0;
    }
    for (int i = 0; i < MAX_FRUTAS; i++) {
        rc->frutas[i].active = 0;
    }
}

// Actualiza el RenderContext a partir del JSON recibido
void updateRenderFromJson(RenderContext* rc, const char* buffer) {
    const char* json = strstr(buffer, "{");
    if (!json) return;

    // --------------------------
    // 1) PLAYER (pantallaObjetivo)
    // --------------------------
    // Buscar sección "players":{
    const char* playersPtr = strstr(json, "\"players\":");
    if (playersPtr) {
        const char* brace = strchr(playersPtr, '{');
        if (brace) {
            char key[64];
            snprintf(key, sizeof(key), "\"%s\":", rc->pantallaObjetivo); // "pantalla1":

            const char* pantObj = strstr(brace, key);
            if (pantObj) {
                const char* objStart = strchr(pantObj, '{');
                if (objStart) {
                    const char* xPtr     = strstr(objStart, "\"x\":");
                    const char* yPtr     = strstr(objStart, "\"y\":");
                    const char* livesPtr = strstr(objStart, "\"lives\":");
                    const char* levelPtr = strstr(objStart, "\"level\":");

                    if (xPtr)   rc->dkjrX = (float)atof(xPtr + 4);
                    if (yPtr)   rc->dkjrY = (float)atof(yPtr + 4);
                    if (livesPtr) rc->lives = atoi(livesPtr + 8);

                    static int lastLevel = -1;
                    if (levelPtr) {
                        int lvl = atoi(levelPtr + 8);
                        if (lastLevel == -1) {
                            lastLevel = lvl;
                            rc->level = lvl;
                        } else {
                            if (lvl > lastLevel) {
                                // Subió de nivel → mostrar WIN
                                rc->winTimer = 1.5f;
                            }
                            lastLevel = lvl;
                            rc->level = lvl;
                        }
                    }

                    rc->hasPlayer = 1;
                }
            } else {
                // No se encontró ese jugador
                rc->hasPlayer = 0;
            }
        }
    }

    // --------------------------
    // 2) OBJETOS (cocodrilos y frutas)
    // --------------------------
    limpiarContextoObjetos(rc);

    const char* objArr = strstr(json, "\"objetos\":[");
    if (!objArr) return;

    objArr += strlen("\"objetos\":[");
    const char* arrEnd = strchr(objArr, ']');
    if (!arrEnd) return;

    int frutaIndex = 0;
    int crocIndex  = 0;

    const char* ptr = objArr;
    while (ptr < arrEnd) {
        const char* objStart = strchr(ptr, '{');
        if (!objStart || objStart >= arrEnd) break;
        const char* objEnd   = strchr(objStart, '}');
        if (!objEnd || objEnd > arrEnd) break;

        char objBuf[512];
        int len = (int)(objEnd - objStart + 1);
        if (len >= (int)sizeof(objBuf)) {
            ptr = objEnd + 1;
            continue;
        }
        strncpy(objBuf, objStart, len);
        objBuf[len] = '\0';

        char tipo[32] = "";
        char pantalla[32] = "";
        float x = 0, y = 0;

        // tipo
        char* tPtr = strstr(objBuf, "\"tipo\":\"");
        if (tPtr) {
            tPtr += strlen("\"tipo\":\"");
            sscanf(tPtr, "%31[^\"]", tipo);
        }

        // x
        char* xPtr = strstr(objBuf, "\"x\":");
        if (xPtr) x = (float)atof(xPtr + 4);

        // y
        char* yPtr = strstr(objBuf, "\"y\":");
        if (yPtr) y = (float)atof(yPtr + 4);

        // pantalla
        char* pPtr = strstr(objBuf, "\"pantalla\":\"");
        if (pPtr) {
            pPtr += strlen("\"pantalla\":\"");
            sscanf(pPtr, "%31[^\"]", pantalla);
        }

        // Solo objetos de la pantalla que estamos observando
        if (strcmp(pantalla, rc->pantallaObjetivo) == 0) {
            if (strcmp(tipo, "rojo") == 0 || strcmp(tipo, "azul") == 0) {
                if (crocIndex < MAX_COCODRILOS) {
                    rc->cocodrilos[crocIndex].x = x;
                    rc->cocodrilos[crocIndex].y = y;
                    rc->cocodrilos[crocIndex].tipo = (strcmp(tipo, "rojo") == 0) ? 0 : 1;
                    rc->cocodrilos[crocIndex].active = 1;
                    crocIndex++;
                }
            } else {
                // Lo tratamos como fruta
                if (frutaIndex < MAX_FRUTAS) {
                    rc->frutas[frutaIndex].x = x;
                    rc->frutas[frutaIndex].y = y;
                    rc->frutas[frutaIndex].active = 1;
                    frutaIndex++;
                }
            }
        }

        ptr = objEnd + 1;
    }
}

// ---------------------------------------------------------------
// MAIN — ESPECTADOR FIJO A UN PLAYER (con menú inicial)
// ---------------------------------------------------------------
int main() {
    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);

    SOCKET sock = conectarServidor(IP, PORT);
    if (sock == INVALID_SOCKET) {
        printf("Error conectando a servidor.\n");
        return 1;
    }

    // Registrarse como spectator
    send(sock, "JOIN_SPECTATOR Spectator\n", 26, 0);
    char ack[64];
    int nAck = recv(sock, ack, sizeof(ack)-1, 0);
    if (nAck <= 0) {
        printf("Error recibiendo ACK del servidor.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    ack[nAck] = '\0';
    printf("[Spectator] Servidor respondió: %s\n", ack);

    RenderContext rc = {0};
    rc.dkjrX = 200;
    rc.dkjrY = 300;
    rc.hasPlayer = 0;
    rc.lives = 3;
    rc.level = 1;
    rc.winTimer = 0.0f;
    strcpy(rc.pantallaObjetivo, "pantalla1"); // valor por defecto

    InitWindow(WIDTH, HEIGHT, "Spectator - DonCEy Kong Jr");
    SetTargetFPS(60);

    // Cargar recursos
    Image fondoImg = LoadImage("client-c/image/Fondo.png");
    Texture2D fondoTexture = LoadTextureFromImage(fondoImg);
    UnloadImage(fondoImg);

    Image playerImg = LoadImage("client-c/image/IPlayer.png");
    Texture2D playerTexture = LoadTextureFromImage(playerImg);
    UnloadImage(playerImg);

    Image redImg = LoadImage("client-c/image/RedCroc.png");
    Texture2D texRedCroc = LoadTextureFromImage(redImg);
    UnloadImage(redImg);

    Image blueImg = LoadImage("client-c/image/BlueCroc.png");
    Texture2D texBlueCroc = LoadTextureFromImage(blueImg);
    UnloadImage(blueImg);

    Image fr1 = LoadImage("client-c/image/fruta1.png");
    Texture2D texFruta1 = LoadTextureFromImage(fr1);
    UnloadImage(fr1);

    Image fr2 = LoadImage("client-c/image/fruta2.png");
    Texture2D texFruta2 = LoadTextureFromImage(fr2);
    UnloadImage(fr2);

    Image fr3 = LoadImage("client-c/image/fruta3.png");
    Texture2D texFruta3 = LoadTextureFromImage(fr3);
    UnloadImage(fr3);

    char buffer[2048];

    // ============================================================
    // MENÚ INICIAL: elegir si observar a Player 1 o Player 2
    // ============================================================
    bool seleccionado = false;
    while (!seleccionado && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("ESPECTADOR - SELECCIONAR JUGADOR", 80, 100, 24, RAYWHITE);
        DrawText("Presione 1 para observar al Player 1", 120, 200, 20, LIGHTGRAY);
        DrawText("Presione 2 para observar al Player 2", 120, 240, 20, LIGHTGRAY);
        DrawText("(La seleccion sera fija, no se puede cambiar luego)", 80, 300, 18, GRAY);

        if (IsKeyPressed(KEY_ONE)) {
            strcpy(rc.pantallaObjetivo, "pantalla1");
            seleccionado = true;
        }
        if (IsKeyPressed(KEY_TWO)) {
            strcpy(rc.pantallaObjetivo, "pantalla2");
            seleccionado = true;
        }

        EndDrawing();
    }

    if (WindowShouldClose()) {
        // Si cerró la ventana en el menú, salir limpio
        CloseWindow();
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    // ============================================================
    // BUCLE PRINCIPAL DE ESPECTADOR (ya con pantalla fija)
    // ============================================================
    while (!WindowShouldClose()) {

        // Leer estado del servidor si hay datos
        int n = recvNoBlock(sock, buffer, sizeof(buffer));
        if (n < 0) break;
        if (n > 0 && strstr(buffer, "STATE")) {
            updateRenderFromJson(&rc, buffer);
        }

        // Actualizar timer de WIN
        float dt = GetFrameTime();
        if (rc.winTimer > 0.0f) {
            rc.winTimer -= dt;
            if (rc.winTimer < 0.0f) rc.winTimer = 0.0f;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexture(fondoTexture, 0, 0, WHITE);

        DrawText("ESPECTADOR (Observer Remoto)", 20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Observando: %s", rc.pantallaObjetivo),
                 20, 50, 18, RAYWHITE);

        if (rc.hasPlayer) {
            // Jugador
            DrawTexture(playerTexture, (int)rc.dkjrX, (int)rc.dkjrY, WHITE);

            // Cocodrilos
            for (int i = 0; i < MAX_COCODRILOS; i++) {
                if (!rc.cocodrilos[i].active) continue;
                Texture2D t = (rc.cocodrilos[i].tipo == 0) ? texRedCroc : texBlueCroc;
                DrawTexture(t, (int)rc.cocodrilos[i].x, (int)rc.cocodrilos[i].y, WHITE);
            }

            // Frutas (rotando texturas)
            for (int i = 0; i < MAX_FRUTAS; i++) {
                if (!rc.frutas[i].active) continue;
                Texture2D t;
                switch (i % 3) {
                    case 0: t = texFruta1; break;
                    case 1: t = texFruta2; break;
                    default: t = texFruta3; break;
                }
                DrawTexture(t, (int)rc.frutas[i].x, (int)rc.frutas[i].y, WHITE);
            }

            // HUD simple
            DrawText(TextFormat("VIDAS: %d", rc.lives), 20, 80, 20, YELLOW);
            DrawText(TextFormat("NIVEL: %d", rc.level), 20, 110, 20, GREEN);

            // WIN!
            if (rc.winTimer > 0.0f) {
                DrawText("WIN!", WIDTH/2 - 80, HEIGHT/2 - 40, 60, YELLOW);
            }

            // GAME OVER
            if (rc.lives <= 0) {
                const char* msg = "GAME OVER";
                int fontSize = 60;
                int textWidth = MeasureText(msg, fontSize);
                DrawText(msg, (WIDTH - textWidth)/2, HEIGHT/2 - 40, fontSize, RED);
            }

        } else {
            const char* msg = "NO HAY JUGADOR CONECTADO PARA ESA PANTALLA";
            int fontSize = 24;
            int textWidth = MeasureText(msg, fontSize);
            DrawText(msg, (WIDTH - textWidth)/2, HEIGHT/2, fontSize, RED);
        }

        EndDrawing();
    }

    // Limpieza
    UnloadTexture(fondoTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(texRedCroc);
    UnloadTexture(texBlueCroc);
    UnloadTexture(texFruta1);
    UnloadTexture(texFruta2);
    UnloadTexture(texFruta3);

    CloseWindow();
    closesocket(sock);
    WSACleanup();
    return 0;
}