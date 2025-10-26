# DonCEy-Kong-Jr
🦍 DonCEy Kong Jr.
Tarea #4 de Paradigmas de Programación (CE3104)
Carrera: Ingeniería en Computadores

🎯 Objetivos del Proyecto
Aplicar los fundamentos del paradigma imperativo mediante el uso de C y del paradigma orientado a objetos con Java.

Diseñar e implementar un juego multijugador inspirado en Donkey Kong Jr. con lógica de servidor y clientes.

Utilizar sockets para la comunicación entre aplicaciones en C (cliente) y Java (servidor).

Crear y manipular listas como estructuras de datos para gestionar entidades del juego.

Implementar mecanismos de administración para crear y eliminar elementos del juego (cocodrilos, frutas).

Garantizar la escalabilidad con hasta 2 jugadores y 2 espectadores por partida.

🖥️ Descripción del Sistema
DonCEy Kong Jr. es un juego multijugador donde los jugadores controlan a Donkey Kong Jr., quien debe escalar plataformas y esquivar cocodrilos para rescatar a su padre.
El sistema se compone de tres módulos principales:

Servidor (Java): Gestiona la lógica del juego, incluyendo la creación de cocodrilos (rojos y azules), frutas, y la validación de movimientos y colisiones.

Cliente Jugador (C): Interfaz gráfica que permite al jugador mover a Donkey Kong Jr. usando el teclado (desplazamiento y salto).

Cliente Espectador (C): Permite observar partidas en tiempo sin interactuar.

La comunicación entre clientes y servidor se realiza mediante sockets, asegurando sincronización en tiempo real.
