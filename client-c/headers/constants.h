// ---------------------------------------------------------------
// Nombre del archivo: constants.h  
// Descripción:
//      Constantes y configuraciones del juego
//      - Límites por partida (cocodrilos, frutas, lianas)
//      - Configuración de red (IP, puerto, máximos)
//      - Parámetros de juego (gravedad, velocidad)
// ---------------------------------------------------------------
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "../lib/raylib/include/raylib.h"

// Constantes de red
#define PORT 5050
#define IP "192.168.124.8"
#define BUFFER_SIZE 2048

// Constantes del juego
#define WIDTH 928
#define HEIGHT 870
#define PLAYER_SPEED 5

// Límites del Juego
#define MAX_PLAYERS 2
#define MAX_OBSERVERS 4
#define MAX_COCODRILOS 4 //Cocodrilos actuales disponibles
#define MAX_FRUTAS 4
#define MAX_LIANAS 6

// Estados del Juego
#define STATE_WAITING 0
#define STATE_PLAYING 1
#define STATE_GAME_OVER 2
#define STATE_VICTORY 3

#endif