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
Texture2D fruta1Texture;
Texture2D fruta2Texture;
Texture2D fruta3Texture;
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

void actualizarElementos(GameState* estado, const char* json, const char* miPantallaId) {
    // Resetear arrays antes de actualizar
    for (int i=0;i<MAX_COCODRILOS;i++) estado->cocodrilos[i].active=false;
    for (int i=0;i<MAX_FRUTAS;i++) estado->frutas[i].active=false;
    // Ubicar el array de "objetos":[ ... ] en json
    char* objetos_start = strstr(json, "\"objetos\":[");
    if (!objetos_start) {
        return;
    }
    objetos_start += strlen("\"objetos\":[");
    char* objetos_end = strchr(objetos_start, ']');
    if (!objetos_end) {
        return;
    }

    // Copiar temporalmente el array de objetos
    int objetos_len = objetos_end - objetos_start;
    char objetos_str[1024];
    strncpy(objetos_str, objetos_start, objetos_len);
    objetos_str[objetos_len] = '\0';

    int obj_count = 0;
    int frutas_creadas = 0;
    int cocodrilos_creados = 0;
    char* obj_token = strtok(objetos_str, "{");

    while (obj_token && obj_count < 10) {
        char tipo[32] = "";
        char pantalla[32] = "";
        int liana = -1;
        float altura = 0.0f;
        float x = 0, y = 0;

        // Extraer tipo
        char* tipo_start = strstr(obj_token, "\"tipo\":\"");
        if (tipo_start) {
            tipo_start += strlen("\"tipo\":\"");
            sscanf(tipo_start, "%31[^\"]", tipo);
        }

        // Extraer x
        char* x_start = strstr(obj_token, "\"x\":");
        if (x_start) sscanf(x_start, "\"x\":%f", &x);

        // Extraer y
        char* y_start = strstr(obj_token, "\"y\":");
        if (y_start) sscanf(y_start, "\"y\":%f", &y);

        // Extraer pantalla
        char* p_start = strstr(obj_token, "\"pantalla\":\"");
        if (p_start) {
            p_start += strlen("\"pantalla\":\"");
            sscanf(p_start, "%31[^\"]", pantalla);
        }

        // Solo agregar si es para mi pantalla
        if (strcmp(pantalla, miPantallaId) == 0) {
            if (strcmp(tipo,"rojo")==0 || strcmp(tipo,"azul")==0 ) {
                // Cocodrilo
                for(int i=0;i<MAX_COCODRILOS;i++) {
                    if (!estado->cocodrilos[i].active) {
                        estado->cocodrilos[i].position = (Vector2){x, y};
                        estado->cocodrilos[i].type = (strcmp(tipo,"red")==0) ? 0 : 1;
                        estado->cocodrilos[i].active = true;
                        printf("Cocodrilo %s creado en (%.1f, %.1f)\n", tipo, x, y);
                        //estado->cocodrilos[i].texture = strcmp(tipo,"red")==0 ? cocodriloRojoTexture : cocodriloAzulTexture;
                        break;
                    }
                }
                } else {
                // Fruta
                for(int i=0;i<MAX_FRUTAS;i++) {
                    if (!estado->frutas[i].active) {
                        estado->frutas[i].position = (Vector2){x, y};
                        estado->frutas[i].active = true;
                        //estado->frutas[i].texture = frutaTexture;
                        break;
                    }
                }
            }
        }
        obj_token = strtok(NULL, "},");
    }

}

// Función para separar json y extraer objetos
int procesarMensajeObjetos(const char* mensaje, GameState* estado, const char* miPantallaId) {
    // Verificar si es un mensaje STATE (formato JSON)
    if (strstr(mensaje, "STATE ") != NULL) {
        // Extraer el JSON después de "STATE "
        char* jsonStart = strstr(mensaje, "STATE ");
        if (jsonStart) {
            jsonStart += 6; // Saltar "STATE "
            actualizarElementos(estado, jsonStart, miPantallaId);
            return 0;
        }
    }
    return -1;
}
        
int registrarJugador(SOCKET sock, const char* nombre){
    char msg[100];
    snprintf(msg, sizeof(msg), "JOIN_PLAYER %s\n", nombre);
    return send(sock, msg, (int)strlen(msg), 0) <= 0; // devuelve 0 si OK, 1 si error
}

int enviarInputs(SOCKET sock, int up, int down, int left, int right, int jump){
    char msg[128];
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
    estado->player.position = (Vector2){100, 500};
    estado->player.width = 0;
    estado->player.height = 60;
    estado->player.score = 0;
    estado->player.texture = playerTexture;

    estado->gameState = 1; // PLAYING
}
void inicializarLianas(Liana lianas[]){}
void inicializarJugador(Player* jugador){}

// ==================== Actualización ====================
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
    fruta1Texture = LoadTexture("client-c/image/fruta1.png");
    fruta2Texture = LoadTexture("client-c/image/fruta2.png");
    fruta3Texture = LoadTexture("client-c/image/fruta3.png");
}

void descargarTexturas(void) {
    UnloadTexture(fondoTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(fruta1Texture);
    UnloadTexture(fruta2Texture);
    UnloadTexture(fruta3Texture);
}

// ==================== Renderizado ====================
void renderizarJuego(const GameState* estado){
    DrawTextureEx(estado->player.texture, estado->player.position, 0.0f, 1.0f, WHITE);
}

void renderizarJugador(const Player* jugador){
    DrawTextureEx(playerTexture, jugador->position, 0.0f, 1.0f, WHITE);
}

void renderizarCocodrilos(const Cocodrilo cocodrilos[], int count){}
void renderizarFrutas(const Fruta frutas[], int count){
    for (int i = 0; i < count; i++) {
        if (frutas[i].active) {
            // Rotar entre las 3 texturas de frutas basado en el índice
            Texture2D fruitTex;
            switch (i % 3) {
                case 0: fruitTex = fruta2Texture; break;
                case 1: fruitTex = fruta2Texture; break;
                case 2: fruitTex = fruta3Texture; break;
                default: fruitTex = fruta1Texture;
            }
            if (fruitTex.id != 0) {
                DrawTextureEx(fruitTex, frutas[i].position, 0.0f, 1.0f, WHITE);
            } else {
                // Fallback si no hay textura
                DrawCircle(frutas[i].position.x + 15, frutas[i].position.y + 15, 15, GREEN);
            //DrawTextureEx(fruitTex, frutas[i].position, 0.0f, 1.0f, WHITE);
            }
        }
    }
}
void renderizarUI(const GameState* instancia){
    DrawTexture(fondoTexture, 0, 0, WHITE);
    DrawTextureEx(instancia->player.texture, instancia->player.position, 0.0f, 1.0f, WHITE);
    //renderizarCocodrilos(instancia->cocodrilos, MAX_COCODRILOS);
    renderizarFrutas(instancia->frutas, MAX_FRUTAS);
    //renderizarJugador(&instancia->player);
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