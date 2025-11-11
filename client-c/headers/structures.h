// ---------------------------------------------------------------
// Nombre del archivo: structures.h  
// Descripción:
//      Define todas las estructuras de datos del juego
//      - Jugador, enemigos, frutas, lianas
//      - Estado del juego (GameInstance)
// ---------------------------------------------------------------
#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "../lib/raylib/include/raylib.h"
#include "constants.h"
#include <stdbool.h>

// Estructura del jugador
typedef struct {
    Vector2 position; //Posicion final
    int width, height; //Dimensiones para render 
    int score; //Puntuacion
    bool connected; //Control del estado
    bool onLiana; //Animacion en liana
    int currentLiana; //Liana actual
    bool bothLiana; //Animacion en dos liana
    bool isJumping; //Animacion de salto
    bool died; //Animacion de muerto
    int currentFrame; //Para animacion
    Texture2D texture;
    float velocityY; //Para cuando se caiga de la liana
} Player;

typedef struct{
    int spriteId;      // Spritesheet
    int framesPerSecond;  // FPS de animación
    int frameCount;       // Número total de frames
    int currentFrame;     // Frame actual
    float timer;          // Tiempo acumulado para animación
} WalkAnimation;

// Estructura de frutas
typedef struct {
    Vector2 position; 
    int currentLiana; //Sobre cual liana?
    int points; //Cuantos puntos vale
    bool active; //Existe
} Fruta;

// Estructura de cocodrilos
typedef struct {
    Vector2 position;
    int type; // 0 = ROJO, 1 = AZUL
    int liana; // Liana asignada
    bool active; //Aun existe
    float speed; // Velocidad de movimiento
    bool movingUp; // Para cocodrilos rojos
} Cocodrilo;

// Estructura de lianas
typedef struct {
    Vector2 top; //Punto inicial
    Vector2 bottom; //Punto final (para la distancia de la liana)
    int id;
} Liana;

// Estado de juego para cada jugador
typedef struct {
    Player player;
    Cocodrilo cocodrilos[MAX_COCODRILOS];
    Fruta frutas[MAX_FRUTAS];
    Liana lianas[MAX_LIANAS];
    float gameSpeed;
    int gameState;               
    bool gameOver;
    int instanceId;  // ID de esta instancia
    bool hasSpectator;   
} GameState;

#endif