// ---------------------------------------------------------------
// Nombre del archivo: client-player
// Paquete: player
// Descripción: 
//      Cliente TCP para juegos LAN/Wi-Fi en Windows
//       - Se conecta a un servidor mediante sockets TCP
//       - Se registra como jugador
//       - Envía inputs 
//
// ACÁ EJECUTAS EL CODIGO PARA CREAR UN PLAYER
// ---------------------------------------------------------------

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#error Solo windows
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>

// IP y PUERTO para modificar
#define PORT 5050
#define IP "127.0.0.1" // localhost

int main() {
    // Inicializar Winsock
    int res;
    WSADATA w; 
    res = WSAStartup(MAKEWORD(2,2), &w);
    if (res){
        printf("Error: %d\n", res);
        return 1;
    }

    // Crear socket cliente
    SOCKET client;
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET){
        printf("Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configurar direccion de IP
    struct sockaddr_in  serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &serv.sin_addr);

    // Conectar al servidor
    if (connect(client, (struct sockaddr*)&serv, sizeof(serv)) != 0) {
        printf("Error al conectar: %d\n", WSAGetLastError());
        closesocket(client);
        WSACleanup();
        return 1;
    }
    printf("[Player] Conectado al servidor %s:%d\n", IP, PORT);

    // Registrarse como jugador
    char buf[2048];
    snprintf(buf, sizeof(buf), "JOIN_PLAYER %s\n", "Player1");
    send(client, buf, (int)strlen(buf), 0);

    // Bucle para la descripcion de cada frame al server
    int tick = 0;
    while (1) {
        int up=0,down=0,left=0,right=0,jump=0;
        right = (tick % 60) < 30 ? 1 : 0; // demo movimiento
        snprintf(buf, sizeof(buf), "INPUT %d %d %d %d %d %d\n",
                 tick, up, down, left, right, jump);
        send(client, buf, (int)strlen(buf), 0);

        int n = recv(client, buf, sizeof(buf)-1, 0);

        // Recibe respuesta del server
        if (n > 0) { buf[n]=0; printf("Servidor: %s\n", buf); }

        Sleep(16);
        tick++;
    }

    // Cerrar socket y limpiar Winsock
    closesocket(client);
    WSACleanup();
    return 0;
}
