// ---------------------------------------------------------------
// Nombre del archivo: player.c
// Descripción:
//      Funciones del cliente jugador
//      - Conexión TCP con servidor
//      - Movimiento y envío de inputs
//      - Renderizado simple con Raylib
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
#error Solo Windows
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../headers/player.h"
#include "../../headers/structures.h"  
#include "../../headers/constants.h"

Texture2D fondoTexture;
Texture2D playerTexture;
Texture2D frutaTexture;
Texture2D cocodriloRojoTexture;
Texture2D cocodriloAzulTexture;
Texture2D lianaTexture;

SOCKET conectarServidor(const char* ip, int port){
    // Crear socket
    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET){
        printf("Error: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }
    
    // Configurar direccion de IP
    struct sockaddr_in  serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv.sin_addr);

    // Conectar al servidor
    if (connect(client, (struct sockaddr*)&serv, sizeof(serv)) == 0) {
        printf("[Player] Conectado al servidor %s:%d\n", ip, port);
        return client;
    } else {
        printf("Error al conectar: %d\n", WSAGetLastError());
        closesocket(client);
        return INVALID_SOCKET;
    }
}

void desconectarServidor(SOCKET sock){
    if (sock != INVALID_SOCKET){
        closesocket(sock);
    }
}

int enviarMensaje(SOCKET sock, const char* mensaje){
    return send(sock, mensaje, (int)strlen(mensaje), 0);
}

int recibirMensaje(SOCKET sock, char* buffer, int bufferSize){
        // Recibir estado de juego 
        fd_set set;
        struct timeval timeout;
        FD_ZERO(&set);
        FD_SET(sock, &set);
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        // Recibe respuesta del server
        if (select(0, &set, NULL, NULL, &timeout) > 0) {
            int n = recv(sock, buffer, bufferSize - 1, 0);
            if (n > 0) { 
                buffer[n] = '\0'; 
                printf("Servidor: %s\n", buffer);
                return n;
            }
        }
        return 0;
}
        
int registrarJugador(SOCKET sock, const char* nombre){
    char msg[100];
    snprintf(msg, sizeof(msg), "JOIN_PLAYER %s\n", nombre);
    return send(sock, msg, (int)strlen(msg), 0) <= 0; // devuelve 0 si OK, 1 si error
}

int enviarInputs(SOCKET sock, int up, int down, int left, int right, int jump){
    char msg[64];
    snprintf(msg, sizeof(msg), "INPUT:%d:%d:%d:%d:%d", up, down, left, right, jump);
    return enviarMensaje(sock, msg);
}

int respuestaServidor(const char* mensaje, GameState* estado){
    float x, y;
    int score;

    // Ejemplo de mensaje del servidor: "POS:200:400:SCORE:50"
    if (sscanf(mensaje, "POS:%f:%f:SCORE:%d", &x, &y, &score) == 3) {
        estado->player.position.x = x;
        estado->player.position.y = y;
        estado->player.score = score;
        return 0;
    }

    printf("[Servidor] Mensaje desconocido: %s\n", mensaje);
    return -1;
}

// ==================== Inicialización ====================
void inicializarGameState(GameState* estado){
    memset(estado, 0, sizeof(GameState));
    
    // Jugador inicial
    estado->player.position = (Vector2){0, 0};
    estado->player.width = 0;
    estado->player.height = 60;
    estado->player.score = 0;
    estado->player.texture = playerTexture;

    // Lianas básicas
    for (int i = 0; i < MAX_LIANAS; i++) {
        estado->lianas[i].top = (Vector2){150 + i * 150, 100};
        estado->lianas[i].bottom = (Vector2){150 + i * 150, 700};
        estado->lianas[i].id = i;
    }

    estado->gameState = 1; // PLAYING
}
void inicializarLianas(Liana lianas[]){}
void inicializarJugador(Player* jugador){}

// ==================== Actualización ====================
void aplicarGravedad(Player* jugador){
    if (!jugador->onLiana) {
        jugador->velocityY += 0.5f;
        jugador->position.y += jugador->velocityY;
        
        // Limitar caída al piso
        if (jugador->position.y > 600) {
            jugador->position.y = 600;
            jugador->velocityY = 0;
        }
    }
}
void actualizarAnimacionJugador(Player* jugador){
}

// ==================== Colisión ====================
bool colisionCocodrilo(const Player* jugador, const Cocodrilo* cocodrilo){
    // Implementación temporal
    return false;
}
bool colisionFruta(Player* jugador, const Fruta* fruta){
    // Implementación temporal
    return false;
}
bool caidaAbismo(const Player* jugador){
    // Implementación temporal
    return false;
}
bool estaEnLiana(const Player* jugador, const Liana* liana){
    // Implementación temporal
    return false;
}

// ==================== CARGA DE TEXTURAS ====================
void cargarTexturas(void) {
    fondoTexture = LoadTexture("client-c/image/Fondo.png");
    playerTexture = LoadTexture("client-c/image/IPlayer.png");
}

void descargarTexturas(void) {
    UnloadTexture(fondoTexture);
    UnloadTexture(playerTexture);
}

// ==================== Renderizado ====================
void renderizarJuego(const GameState* estado){
    // Dibujar lianas
    for (int i = 0; i < MAX_LIANAS; i++){
        DrawLineV(estado->lianas[i].top, estado->lianas[i].bottom, BROWN);
    }
    DrawTextureEx(estado->player.texture, estado->player.position, 0.0f, 1.0f, WHITE);
}

void renderizarJugador(const Player* jugador){
    DrawTextureEx(playerTexture, jugador->position, 0.0f, 1.0f, WHITE);
}

void renderizarCocodrilos(const Cocodrilo cocodrilos[], int count){}
void renderizarFrutas(const Fruta frutas[], int count){}
void renderizarLianas(const Liana lianas[], int count){}
void renderizarUI(const GameState* instancia){
    renderizarLianas(instancia->lianas, MAX_LIANAS);
    renderizarCocodrilos(instancia->cocodrilos, MAX_COCODRILOS);
    renderizarFrutas(instancia->frutas, MAX_FRUTAS);
    renderizarJugador(&instancia->player);
}

// ==================== Utilidades ====================
float obtenerDeltaTime(void){
    static double lastTime = 0.0;
    double currentTime = GetTime();
    float delta = (float)(currentTime - lastTime);
    lastTime = currentTime;
    
    if (delta > 0.1f) delta = 0.1f; 
    return delta;
}
Vector2 calcularPosicionEnLiana(int lianaId, float altura){return (Vector2){0,0};}
bool estaDentroPantalla(const Vector2* posicion){ return true; }
void reiniciarJuego(GameState* instancia){}