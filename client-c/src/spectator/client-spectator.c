// ---------------------------------------------------------------
// Nombre del archivo: client-spectator
// Descripción: 
//      Cliente TCP espectador para juegos LAN/Wi-Fi en Windows
//       - Se conecta a un servidor mediante sockets TCP
//       - Se registra como espectador
//       - Recibe estado del juego
// ---------------------------------------------------------------
// Solo hace JOIN_SPECTATOR y lee STATE para dibujar.
// ---------------------------------------------------------------
// Nombre del archivo: client-spectator.c
// Descripción:
//      Cliente TCP espectador con interfaz gráfica en Raylib.
//      - Se conecta al servidor
//      - Se registra como espectador
//      - Muestra una ventana blanca (futuro: renderiza estado)
// ---------------------------------------------------------------


#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX

#include "../lib/raylib/include/raylib.h"

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
// CONFIGURACIÓN (ajusta según tu red)
// -------------------------------
#define IP "127.0.0.1"
#define PORT 5050
#define WIDTH 800
#define HEIGHT 600

// -------------------------------
// FUNCIONES DE RED (autocontenidas)
// -------------------------------

SOCKET conectarServidor(const char* ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
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

int recibirMensaje(SOCKET sock, char* buffer, int bufferSize) {
    fd_set readSet;
    struct timeval timeout = {0, 10000}; // 10 ms
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    if (select(0, &readSet, NULL, NULL, &timeout) <= 0) {
        return 0; // No hay datos o timeout
    }

    int n = recv(sock, buffer, bufferSize - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        return n;
    }
    return -1; // Conexión cerrada
}

// -------------------------------
// FUNCIÓN PRINCIPAL
// -------------------------------

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
        printf("[Spectator] Error al conectar al servidor %s:%d (Error: %d)\n", IP, PORT, WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("[Spectator] Conectado al servidor %s:%d\n", IP, PORT);

    // Registrar como espectador
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
        printf("[Spectator] Registro rechazado: %s\n", bytes > 0 ? ackBuffer : "(sin respuesta)");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    printf("[Spectator] Registro exitoso. Modo observador activo.\n");

    // Iniciar ventana gráfica
    InitWindow(WIDTH, HEIGHT, "DonCEy Kong Jr - Espectador");
    SetTargetFPS(60);

    char buffer[2048];

    // Bucle principal: solo recibe estado
    while (!WindowShouldClose()) {
        int n = recibirMensaje(serverSocket, buffer, sizeof(buffer));
        if (n > 0) {
            // Opcional: parsear JSON aquí en el futuro
            printf("Estado: %s", buffer);
        } else if (n < 0) {
            printf("[Spectator] Servidor cerró la conexión.\n");
            break;
        }

        // Renderizado
        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("ESPECTADOR - DonCEy Kong Jr", 20, 20, 20, DARKGRAY);
        DrawText("Observando la partida...", 20, 50, 16, GRAY);
        EndDrawing();
    }

    // Limpiar
    CloseWindow();
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}