# TP2 - Tecnologia Digital II (Tetris con memoria dinamica)

Integrantes: Delfina Santander, Antonia Schereschevsky, Ana Catalina Romero

Refactor del Tetris base: el tablero pasa a ser una lista enlazada de filas, y la cola de piezas y las particulas son listas sin tamano fijo.

Necesita SDL2 y valgrind:

    sudo apt-get install build-essential libsdl2-dev valgrind

Uso:

    make            # compila ./tetris
    make run        # corre los tests y despues abre el juego
    make test       # solo tests, sin ventana
    make valgrind   # tests bajo valgrind

Todo el codigo esta en `tp_resuelto.c`. Controles: flechas para mover/rotar, espacio para caida rapida.
