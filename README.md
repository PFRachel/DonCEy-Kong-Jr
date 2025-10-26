# 🦍 DonCEy Kong Jr.
### Tarea #4 de Paradigmas de Programación (CE3104)  
**Carrera:** Ingeniería en Computadores  

---

## 🎯 Objetivos del Proyecto

- Aplicar **paradigma imperativo** en C y **orientado a objetos** en Java
- Implementar juego multijugador inspirado en *Donkey Kong Jr.*
- Utilizar **sockets** para comunicación cliente-servidor
- Gestionar entidades del juego mediante **listas** como estructuras de datos
- Soportar hasta **2 jugadores** y **2 espectadores** por partida

---

## 🖥️ Descripción del Sistema

**DonCEy Kong Jr.** es un juego multijugador donde los jugadores controlan a *Donkey Kong Jr.* para rescatar a su padre, esquivando cocodrilos y recolectando frutas.

### Módulos Principales:

- **Servidor (Java):** Lógica del juego, creación de cocodrilos (rojos/azules) y frutas
- **Cliente Jugador (C):** Interfaz gráfica con controles de teclado
- **Cliente Espectador (C):** Visualización en tiempo real

**Comunicación:** Sockets entre C y Java para sincronización en red.
