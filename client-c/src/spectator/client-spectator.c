// ---------------------------------------------------------------
// Nombre del archivo: client-spectator.c
// Descripción:
//   Cliente TCP ESPECTADOR con Raylib.
//   - TODO el patrón Observer está en Java.
//   - Este cliente SOLO recibe JSON "STATE {...}" por TCP
//     y dibuja el estado gráfico (mono, cocodrilos, frutas).
//   - Permite cambiar la pantalla observada (pantalla1 / pantalla2)
//     con las teclas 1 y 2.
//   - También muestra WIN y GAME OVER basándose en lives y level.
// ---------------------------------------------------------------

// Evitar conflictos entre Raylib y Windows
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX

#include "../../lib/raylib/include/raylib.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#error Solo Windows
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../headers/constants.h"
#include "../../headers/structures.h"

// ---------------------------------------------------------------
// RenderContext: datos gráficos que se actualizan al recibir STATE
// ---------------------------------------------------------------
typedef struct RenderContext {
    float dkjrX;
    float dkjrY;
    int   hasPlayer;                     // 1 = hay jugador, 0 = no

    char pantallaObjetivo[32];           // "pantalla1" o "pantalla2"

    // Objetos visibles en ESA pantalla
    Cocodrilo cocodrilos[MAX_COCODRILOS];
    Fruta     frutas[MAX_FRUTAS];

    // Estado lógico del jugador observado
    int lives;
    int level;
    int winFrames;                       // frames para mostrar "WIN!"
} RenderContext;

// ---------------------------------------------------------------
// Conexión TCP básica (cliente) → servidor Java
// ---------------------------------------------------------------
SOCKET conectarServidor(const char* ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Error creando socket: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        printf("Error en inet_pton\n");
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Error conectando al servidor: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

// ---------------------------------------------------------------
// Recibir sin bloquear usando select()
// ---------------------------------------------------------------
int recvNoBlock(SOCKET sock, char* buf, int size) {
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(sock, &set);

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;  // 1 ms

    int sel = select(0, &set, NULL, NULL, &timeout);
    if (sel <= 0) {
        return 0; // nada que leer
    }

    int n = recv(sock, buf, size - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
    }
    return n;
}

// ---------------------------------------------------------------
// Actualizar RenderContext a partir del JSON STATE recibido
//  - NO es un patrón Observer aquí: solo parsing gráfico.
//  - Java ya aplicó el patrón Observer y nos manda el JSON.
// ---------------------------------------------------------------
void updateRenderFromJson(RenderContext* rc, const char* buffer, const char* pantallaIdDestino) {
    // Guardar la pantalla objetivo en el contexto
    strncpy(rc->pantallaObjetivo, pantallaIdDestino, sizeof(rc->pantallaObjetivo) - 1);
    rc->pantallaObjetivo[sizeof(rc->pantallaObjetivo) - 1] = '\0';

    // Por defecto, asumimos que NO hay jugador para esa pantalla
    rc->hasPlayer = 0;

    // Resetear crocs y frutas
    for (int i = 0; i < MAX_COCODRILOS; i++) {
        rc->cocodrilos[i].active = false;
    }
    for (int i = 0; i < MAX_FRUTAS; i++) {
        rc->frutas[i].active = false;
    }

    const char* json = strstr(buffer, "{");
    if (!json) return;

    // ============================
    // 1) PLAYER (pos x,y, lives, level)
    // ============================
    char key[64];
    snprintf(key, sizeof(key), "\"%s\":", pantallaIdDestino);

    const char* pantallaObj = strstr(json, key);
    if (pantallaObj) {
        const char* objStart = strchr(pantallaObj, '{');
        if (objStart) {
            const char* xPtr     = strstr(objStart, "\"x\":");
            const char* yPtr     = strstr(objStart, "\"y\":");
            const char* livesPtr = strstr(objStart, "\"lives\":");
            const char* levelPtr = strstr(objStart, "\"level\":");

            if (xPtr) rc->dkjrX = (float)atof(xPtr + 4);
            if (yPtr) rc->dkjrY = (float)atof(yPtr + 4);

            if (livesPtr) {
                rc->lives = atoi(livesPtr + 8); // "lives": -> 8 chars
            }

            if (levelPtr) {
                int newLevel = atoi(levelPtr + 8);
                if (newLevel > rc->level) {
                    // Nivel subió → mostrar WIN un rato
                    rc->winFrames = 90; // ~1.5s @60fps
                }
                rc->level = newLevel;
            }

            rc->hasPlayer = 1;
        }
    }

    // ============================
    // 2) OBJETOS (crocs + frutas)
    // ============================
    const char* objsStart = strstr(json, "\"objetos\":[");
    if (!objsStart) return;

    objsStart += strlen("\"objetos\":[");
    const char* objsEnd = strchr(objsStart, ']');
    if (!objsEnd) return;

    int len = (int)(objsEnd - objsStart);
    char* objsBuf = (char*)malloc(len + 1);
    if (!objsBuf) return;

    strncpy(objsBuf, objsStart, len);
    objsBuf[len] = '\0';

    char* current = objsBuf;
    int crocIndex = 0;
    int frutaIndex = 0;

    while (*current) {
        char* obj_start = strchr(current, '{');
        if (!obj_start) break;

        char* obj_end = strchr(obj_start + 1, '}');
        if (!obj_end) break;

        int obj_len = (int)(obj_end - obj_start + 1);
        if (obj_len <= 0 || obj_len >= 512) {
            current = obj_end + 1;
            continue;
        }

        char obj[512];
        strncpy(obj, obj_start, obj_len);
        obj[obj_len] = '\0';

        char tipo[32] = "";
        char pantalla[32] = "";
        float x = 0.0f, y = 0.0f;

        // tipo
        char* tipo_start = strstr(obj, "\"tipo\":\"");
        if (tipo_start) {
            tipo_start += 8; // strlen("\"tipo\":\"")
            sscanf(tipo_start, "%31[^\"]", tipo);
        }

        // x
        char* x_start = strstr(obj, "\"x\":");
        if (x_start) {
            sscanf(x_start + 4, "%f", &x);
        }

        // y
        char* y_start = strstr(obj, "\"y\":");
        if (y_start) {
            sscanf(y_start + 4, "%f", &y);
        }

        // pantalla
        char* p_start = strstr(obj, "\"pantalla\":\"");
        if (p_start) {
            p_start += 12; // strlen("\"pantalla\":\"")
            sscanf(p_start, "%31[^\"]", pantalla);
        }

        // Solo objetos de la pantalla que estamos observando
        if (strcmp(pantalla, pantallaIdDestino) == 0) {
            // Cocodrilos: tipo "rojo" / "azul"
            if ((strcmp(tipo, "rojo") == 0 || strcmp(tipo, "azul") == 0) &&
                crocIndex < MAX_COCODRILOS) {

                rc->cocodrilos[crocIndex].position.x = x;
                rc->cocodrilos[crocIndex].position.y = y;
                rc->cocodrilos[crocIndex].type = (strcmp(tipo, "rojo") == 0) ? 0 : 1; // 0=rojo,1=azul
                rc->cocodrilos[crocIndex].active = true;
                crocIndex++;

            } else if (frutaIndex < MAX_FRUTAS) {
                // Tratamos otros tipos como fruta ("FRUTA")
                rc->frutas[frutaIndex].position.x = x;
                rc->frutas[frutaIndex].position.y = y;
                rc->frutas[frutaIndex].active = true;
                frutaIndex++;
            }
        }

        current = obj_end + 1;
    }

    free(objsBuf);
}

// ---------------------------------------------------------------
// MAIN — Cliente Espectador (solo gráfico con sprites)
// ---------------------------------------------------------------
int main() {
    WSADATA w;
    if (WSAStartup(MAKEWORD(2,2), &w) != 0) {
        printf("Error en WSAStartup: %d\n", WSAGetLastError());
        return 1;
    }

    // Pantalla inicial que se quiere observar
    char pantallaObjetivo[32] = "pantalla1";

    SOCKET sock = conectarServidor(IP, PORT);
    if (sock == INVALID_SOCKET) {
        printf("Error conectando a servidor.\n");
        WSACleanup();
        return 1;
    }

    // Registrarse como spectator
    send(sock, "JOIN_SPECTATOR Spectator\n", 26, 0);

    char ack[64];
    int nAck = recv(sock, ack, sizeof(ack) - 1, 0);
    if (nAck <= 0) {
        printf("Error recibiendo ACK del servidor.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    ack[nAck] = '\0';

    if (strncmp(ack, "ACK", 3) == 0) {
        // Formato esperado: "ACK pantalla1" o "ACK pantalla2"
        char target[32];
        if (sscanf(ack, "ACK %31s", target) == 1) {
            strcpy(pantallaObjetivo, target);
        }
        printf("[Spectator] Asignado a observar: %s\n", pantallaObjetivo);
    } else {
        printf("Servidor no aceptó spectator: %s\n", ack);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Contexto gráfico
    RenderContext rc;
    memset(&rc, 0, sizeof(RenderContext));
    strcpy(rc.pantallaObjetivo, pantallaObjetivo);
    rc.lives = 3;
    rc.level = 1;
    rc.winFrames = 0;

    // --- Inicializar ventana Raylib ---
    InitWindow(WIDTH, HEIGHT, "Spectator - DonCEy Kong Jr");
    SetTargetFPS(60);

    // ----------------- CARGA DE TEXTURAS -----------------
    // Fondo
    Image fondoImg = LoadImage("client-c/image/Fondo.png");
    Texture2D fondoTexture = LoadTextureFromImage(fondoImg);
    UnloadImage(fondoImg);

    // Sprite del jugador
    Image playerImg = LoadImage("client-c/image/IPlayer.png");
    Texture2D playerTexture = LoadTextureFromImage(playerImg);
    UnloadImage(playerImg);

    // Cocodrilos
    Image redImg = LoadImage("client-c/image/RedCroc.png");
    Texture2D redCrocTexture = LoadTextureFromImage(redImg);
    UnloadImage(redImg);

    Image blueImg = LoadImage("client-c/image/BlueCroc.png");
    Texture2D blueCrocTexture = LoadTextureFromImage(blueImg);
    UnloadImage(blueImg);

    // Frutas
    Image fruta1Img = LoadImage("client-c/image/fruta1.png");
    Texture2D fruta1Texture = LoadTextureFromImage(fruta1Img);
    UnloadImage(fruta1Img);

    Image fruta2Img = LoadImage("client-c/image/fruta2.png");
    Texture2D fruta2Texture = LoadTextureFromImage(fruta2Img);
    UnloadImage(fruta2Img);

    Image fruta3Img = LoadImage("client-c/image/fruta3.png");
    Texture2D fruta3Texture = LoadTextureFromImage(fruta3Img);
    UnloadImage(fruta3Img);

    char buffer[2048];

    // -----------------------------------------------------------
    // Bucle principal
    // -----------------------------------------------------------
    while (!WindowShouldClose()) {

        // Cambiar pantalla observada con teclas 1 y 2
        if (IsKeyPressed(KEY_ONE)) {
            strcpy(pantallaObjetivo, "pantalla1");
        }
        if (IsKeyPressed(KEY_TWO)) {
            strcpy(pantallaObjetivo, "pantalla2");
        }

        // Leer datos del servidor (si hay)
        int n = recvNoBlock(sock, buffer, sizeof(buffer));
        if (n < 0) break;              // error
        if (n > 0 && strstr(buffer, "STATE")) {
            // Actualizar contexto gráfico desde el JSON
            updateRenderFromJson(&rc, buffer, pantallaObjetivo);
        }

        // ---------------- DIBUJAR ----------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(fondoTexture, 0, 0, WHITE);

        DrawText("ESPECTADOR (Observer en Java)", 20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Observando: %s (teclas 1 y 2)", pantallaObjetivo),
                 20, 50, 18, RAYWHITE);

        // Mono
        if (rc.hasPlayer) {
            DrawTexture(playerTexture, (int)rc.dkjrX, (int)rc.dkjrY, WHITE);
        } else {
            const char* msg = "NO HAY JUGADOR CONECTADO";
            int fontSize = 30;
            int textWidth = MeasureText(msg, fontSize);
            DrawText(msg, (WIDTH - textWidth) / 2, HEIGHT / 2, fontSize, RED);
        }

        // Cocodrilos (con sprites)
        for (int i = 0; i < MAX_COCODRILOS; i++) {
            if (!rc.cocodrilos[i].active) continue;
            float x = rc.cocodrilos[i].position.x;
            float y = rc.cocodrilos[i].position.y;

            Texture2D tex = (rc.cocodrilos[i].type == 0) 
                            ? redCrocTexture   // 0 = rojo
                            : blueCrocTexture; // 1 = azul

            DrawTexture(tex, (int)x, (int)y, WHITE);
        }

        // Frutas (rotando entre 3 sprites)
        for (int i = 0; i < MAX_FRUTAS; i++) {
            if (!rc.frutas[i].active) continue;

            float x = rc.frutas[i].position.x;
            float y = rc.frutas[i].position.y;

            Texture2D fruitTex;
            switch (i % 3) {
                case 0: fruitTex = fruta1Texture; break;
                case 1: fruitTex = fruta2Texture; break;
                default: fruitTex = fruta3Texture; break;
            }

            DrawTexture(fruitTex, (int)x, (int)y, WHITE);
        }

        // WIN (cuando sube de nivel)
        if (rc.winFrames > 0) {
            rc.winFrames--;
            const char* msg = "WIN!";
            int fontSize = 60;
            int textWidth = MeasureText(msg, fontSize);
            DrawText(msg, (WIDTH - textWidth) / 2, 200, fontSize, YELLOW);
        }

        // GAME OVER (cuando vidas llegan a 0)
        if (rc.hasPlayer && rc.lives <= 0) {
            const char* msg = "GAME OVER";
            int fontSize = 60;
            int textWidth = MeasureText(msg, fontSize);
            DrawText(msg, (WIDTH - textWidth) / 2, HEIGHT / 2 - 30, fontSize, RED);
        }

        EndDrawing();
    }

    // Limpieza
    UnloadTexture(fondoTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(redCrocTexture);
    UnloadTexture(blueCrocTexture);
    UnloadTexture(fruta1Texture);
    UnloadTexture(fruta2Texture);
    UnloadTexture(fruta3Texture);

    CloseWindow();
    closesocket(sock);
    WSACleanup();
    return 0;
}
