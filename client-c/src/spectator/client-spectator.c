// ---------------------------------------------------------------
// Nombre del archivo: client-spectator.c
// Descripción:
//   Cliente TCP ESPECTADOR con Raylib usando patrón OBSERVER.
//   - NetworkSubject recibe el estado del servidor (STATE {...})
//   - Observers reaccionan cuando hay nuevo GameStateC
//   - Este cliente SOLO observa, no envía INPUTs
// ---------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX

#include "../../lib/raylib/include/raylib.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #error Este cliente solo funciona en Windows.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -------------------------------
// CONFIGURACIÓN
// -------------------------------
#define IP     "127.0.0.1"
#define PORT   5050
#define WIDTH  800
#define HEIGHT 600
#define MAX_OBSERVERS 8

// -------------------------------
// ESTADO DEL JUEGO (lado cliente)
// -------------------------------
typedef struct GameStateC {
    float dkjrX;
    float dkjrY;
    // Futuro: crocs, frutas, etc.
} GameStateC;

// -------------------------------
// OBSERVER
// -------------------------------
typedef void (*StateUpdateFn)(const GameStateC* state, void* userData);

typedef struct {
    StateUpdateFn callback;
    void* userData;
} StateObserver;

// -------------------------------
// SUBJECT: red + último estado
// -------------------------------
typedef struct {
    SOCKET sock;
    GameStateC state;
    StateObserver observers[MAX_OBSERVERS];
    int observerCount;
} NetworkSubject;

// -------------------------------
// TEXTURAS
// -------------------------------
Texture2D fondoTexture;
Texture2D playerTexture;

// ===============================================================
// FUNCIONES DE RED
// ===============================================================
SOCKET conectarServidor(const char* ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }
    return sock;
}

int enviarMensaje(SOCKET sock, const char* mensaje) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "%s\n", mensaje);
    if (len <= 0) return -1;
    return send(sock, buf, len, 0) <= 0 ? -1 : 0;
}

int recibirMensajeNoBloqueante(SOCKET sock, char* buffer, int bufferSize) {
    fd_set readSet;
    struct timeval timeout;

    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000; // 10 ms

    if (select(0, &readSet, NULL, NULL, &timeout) <= 0) {
        return 0; // sin datos
    }

    int n = recv(sock, buffer, bufferSize - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        return n;
    }
    return -1; // conexión cerrada o error
}

// ===============================================================
// SUBJECT: manejo de observers
// ===============================================================
void NetworkSubject_init(NetworkSubject* subj, SOCKET sock) {
    subj->sock = sock;
    subj->observerCount = 0;

    // Estado inicial
    subj->state.dkjrX = 200.0f;
    subj->state.dkjrY = 300.0f;
}

int NetworkSubject_addObserver(NetworkSubject* subj,
                               StateUpdateFn cb,
                               void* userData) {
    if (subj->observerCount >= MAX_OBSERVERS) return -1;
    subj->observers[subj->observerCount].callback = cb;
    subj->observers[subj->observerCount].userData = userData;
    subj->observerCount++;
    return 0;
}

void NetworkSubject_notify(NetworkSubject* subj) {
    for (int i = 0; i < subj->observerCount; i++) {
        subj->observers[i].callback(&subj->state,
                                    subj->observers[i].userData);
    }
}

// Parseo muy simple: buscamos "dkjr":{"x":...,"y":...}
static void NetworkSubject_parseAndUpdate(NetworkSubject* subj,
                                          const char* buffer) {
    // Mensaje típico: STATE {"tick":...,"dkjr":{"x":166.6,"y":400.0},...}
    const char* json = strstr(buffer, "{");
    if (!json) return;

    const char* dk = strstr(json, "\"dkjr\"");
    if (!dk) return;

    const char* xPtr = strstr(dk, "\"x\":");
    const char* yPtr = strstr(dk, "\"y\":");

    if (xPtr) {
        subj->state.dkjrX = (float)atof(xPtr + 4); // 4 = strlen("\"x\":")
    }
    if (yPtr) {
        subj->state.dkjrY = (float)atof(yPtr + 4);
    }
}

// Llamar esto cada frame para ver si hay nuevo STATE
int NetworkSubject_poll(NetworkSubject* subj) {
    char buffer[2048];
    int n = recibirMensajeNoBloqueante(subj->sock, buffer, sizeof(buffer));
    if (n > 0) {
        buffer[n] = '\0';
        printf("Estado: %s\n", buffer);
        NetworkSubject_parseAndUpdate(subj, buffer);
        NetworkSubject_notify(subj);
    }
    return n;
}

// ===============================================================
// OBSERVER DE RENDER
// ===============================================================
typedef struct RenderContext {
    float dkjrX;
    float dkjrY;
} RenderContext;

void onStateUpdateRender(const GameStateC* state, void* userData) {
    RenderContext* rc = (RenderContext*)userData;
    rc->dkjrX = state->dkjrX;
    rc->dkjrY = state->dkjrY;
}

// ===============================================================
// MAIN
// ===============================================================
int main() {
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("[Spectator] Error al iniciar Winsock.\n");
        return 1;
    }

    // Conectar al servidor
    SOCKET serverSocket = conectarServidor(IP, PORT);
    if (serverSocket == INVALID_SOCKET) {
        printf("[Spectator] Error al conectar al servidor %s:%d (Error: %d)\n",
               IP, PORT, WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("[Spectator] Conectado al servidor %s:%d\n", IP, PORT);

    // Registrar como ESPECTADOR
    if (enviarMensaje(serverSocket, "JOIN_SPECTATOR Spectator") != 0) {
        printf("[Spectator] Error al enviar JOIN_SPECTATOR.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Esperar ACK
    char ackBuffer[64];
    int bytes = recv(serverSocket, ackBuffer, sizeof(ackBuffer) - 1, 0);
    if (bytes <= 0 || strstr(ackBuffer, "ACK") == NULL) {
        printf("[Spectator] Registro rechazado: %s\n",
               (bytes > 0) ? ackBuffer : "(sin respuesta)");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    printf("[Spectator] Registro exitoso. Modo observador activo.\n");

    // Crear SUBJECT de red
    NetworkSubject subject;
    NetworkSubject_init(&subject, serverSocket);

    // Crear contexto de render y registrarlo como OBSERVER
    RenderContext renderCtx;
    renderCtx.dkjrX = 200.0f;
    renderCtx.dkjrY = 300.0f;
    NetworkSubject_addObserver(&subject, onStateUpdateRender, &renderCtx);

    // Iniciar ventana Raylib
    InitWindow(WIDTH, HEIGHT, "DonCEy Kong Jr - Espectador");
    SetTargetFPS(60);

    // Cargar texturas (mismas rutas que tu cliente jugador)
    Image fondoImg = LoadImage("client-c/image/Fondo.png");
    fondoTexture = LoadTextureFromImage(fondoImg);
    UnloadImage(fondoImg);

    Image playerImg = LoadImage("client-c/image/IPlayer.png");
    playerTexture = LoadTextureFromImage(playerImg);
    UnloadImage(playerImg);

    if (fondoTexture.id == 0)  printf("[Spectator] ERROR cargando Fondo.png\n");
    if (playerTexture.id == 0) printf("[Spectator] ERROR cargando IPlayer.png\n");

    // Bucle principal
    while (!WindowShouldClose()) {
        //Preguntar al SUBJECT si hay nuevo estado
        int n = NetworkSubject_poll(&subject);
        if (n < 0) {
            printf("[Spectator] Servidor cerró la conexión.\n");
            break;
        }

        //Renderizar usando el último estado conocido (renderCtx)
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(fondoTexture, 0, 0, WHITE);
        DrawTexture(playerTexture, (int)renderCtx.dkjrX, (int)renderCtx.dkjrY, WHITE);

        DrawText("ESPECTADOR - DonCEy Kong Jr", 20, 20, 20, DARKGRAY);
        DrawText("Observando la partida (Observer Pattern)", 20, 50, 16, GRAY);

        EndDrawing();
    }

    // Limpiar
    UnloadTexture(fondoTexture);
    UnloadTexture(playerTexture);
    CloseWindow();
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}