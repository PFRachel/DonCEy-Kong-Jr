// ---------------------------------------------------------------
// Nombre del archivo: client-spectator
// Descripción: 
//      Cliente TCP espectador para juegos LAN/Wi-Fi en Windows
//       - Se conecta a un servidor mediante sockets TCP
//       - Se registra como espectador
//       - Recibe estado del juego
// ---------------------------------------------------------------
// Solo hace JOIN_SPECTATOR y lee STATE para dibujar.
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#error Solo Windows
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 5050
#define IP "127.0.0.1"

int main() {
    // Inicializar Winsock
    WSADATA w;
    if (WSAStartup(MAKEWORD(2,2), &w) != 0) {
        printf("Error al iniciar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    // Crear socket cliente
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Error al crear socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    // Configurar dirección del servidor
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &serv.sin_addr);

    // Conectar al servidor
    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) != 0) {
        printf("Error al conectar: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("[Spectator] Conectado al servidor %s:%d, esperando estado del juego...\n", IP, PORT);

    // Registrarse como espectador
    char buf[2048];
    snprintf(buf, sizeof(buf), "JOIN_SPECTATOR %s\n", "Spectator1");
    send(sock, buf, (int)strlen(buf), 0);

    // Bucle para recibir estado del juego
    while (1) {
        int n = recv(sock, buf, sizeof(buf)-1, 0);
        if (n > 0) {
            buf[n] = 0;
            printf("STATE: %s\n", buf);
            }
        Sleep(16); 
    }

    // Cerrar socket y limpiar Winsock
    closesocket(sock);
    WSACleanup();

    return 0;
}
