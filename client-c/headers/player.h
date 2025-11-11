// ---------------------------------------------------------------
// Nombre del archivo: player.h
// Descripción:
//      Header unificado para el cliente jugador
//      - Conexión TCP con servidor Java
//      - Gestión de inputs y renderizado
//      - Detección de colisiones y física básica
//      - Comunicación mediante sockets Windows
// ---------------------------------------------------------------
#ifndef PLAYER_H
#define PLAYER_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
//#pragma comment(lib, "ws2_32.lib")
#else
#error Solo Windows
#endif

#include "structures.h"
#include "constants.h"

// --- Texturas globales ---
extern Texture2D fondoTexture;
extern Texture2D playerTexture;
extern Texture2D frutaTexture;
extern Texture2D cocodriloRojoTexture;
extern Texture2D cocodriloAzulTexture;
extern Texture2D lianaTexture;

// ==================== Funciones de conexión ====================
SOCKET conectarServidor(const char* ip, int port);
void desconectarServidor(SOCKET sock);
int enviarMensaje(SOCKET sock, const char* mensaje);
int recibirMensaje(SOCKET sock, char* buffer, int bufferSize);
int registrarJugador(SOCKET sock, const char* nombre);
int enviarInputs(SOCKET sock, int up, int down, int left, int right, int jump);
int respuestaServidor(const char* mensaje, GameState* estado);

// ==================== Inicialización ====================
void inicializarGameState(GameState* estado);
void inicializarLianas(Liana lianas[]);
void inicializarJugador(Player* jugador);

// ==================== Actualización ====================
void aplicarGravedad(Player* jugador);
void actualizarAnimacionJugador(Player* jugador);

// ==================== Colisión ====================
bool colisionCocodrilo(const Player* jugador, const Cocodrilo* cocodrilo);
bool colisionFruta(Player* jugador, const Fruta* fruta);
bool caidaAbismo(const Player* jugador);
bool estaEnLiana(const Player* jugador, const Liana* liana);

// ==================== CARGA DE TEXTURAS ====================
void cargarTexturas(void);
void descargarTexturas(void);

// ==================== Renderizado ====================
void renderizarJuego(const GameState* estado);
void renderizarJugador(const Player* jugador);
void renderizarCocodrilos(const Cocodrilo cocodrilos[], int count);
void renderizarFrutas(const Fruta frutas[], int count);
void renderizarLianas(const Liana lianas[], int count);
void renderizarUI(const GameState* instancia);

// ==================== Utilidades ====================
float obtenerDeltaTime(void);
Vector2 calcularPosicionEnLiana(int lianaId, float altura);
bool estaDentroPantalla(const Vector2* posicion);
void reiniciarJuego(GameState* instancia);

#endif