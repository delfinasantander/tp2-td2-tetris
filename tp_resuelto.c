// TP2 - Tecnologia Digital II - UTDT
// Integrantes: Delfina Santander, Antonia Schereschevsky, Ana Catalina Romero
//
// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

// Helper para no repetir el while en todas las funciones: cuenta los
// caracteres hasta el '\0' (el '\0' no se cuenta).
static int largo(char* s) {
    int n = 0;
    while (s[n] != '\0') n++;
    return n;
}

char* strDuplicate(char* src) {
    if (src == NULL) return NULL;
    int n = largo(src);
    char* copia = malloc(n + 1);   // el +1 es para el '\0'
    if (copia == NULL) return NULL;
    // uso <= para que tambien se copie el '\0' del final
    for (int i = 0; i <= n; i++) copia[i] = src[i];
    return copia;
}

int strCompare(char* s1, char* s2) {
    int i = 0;
    // avanzo mientras sean iguales y no se haya terminado s1.
    // si s1 termina, s1[i] vale '\0' y ya no tiene sentido seguir
    while (s1[i] != '\0' && s1[i] == s2[i]) i++;

    // unsigned char igual que hace strcmp, para que los chars > 127 no den negativos
    unsigned char a = (unsigned char)s1[i];
    unsigned char b = (unsigned char)s2[i];

    if (a == b) return 0;      // llegaron los dos al '\0' a la vez
    // un prefijo queda como menor solo: '\0' (0) es menor que cualquier otro char
    return (a < b) ? 1 : -1;   // ojo que es al reves que strcmp
}

char* strConcatenate(char* src1, char* src2) {
    if (src1 == NULL || src2 == NULL) return NULL;
    int n1 = largo(src1);
    int n2 = largo(src2);

    char* res = malloc(n1 + n2 + 1);
    if (res == NULL) return NULL;   // si falla no libero nada, el que llamo sigue siendo dueño de los dos

    for (int i = 0; i < n1; i++) res[i] = src1[i];
    for (int i = 0; i <= n2; i++) res[n1 + i] = src2[i];   // <= para copiar el '\0'

    // recien ahora, con el resultado ya armado, se pueden liberar los originales
    free(src1);
    free(src2);
    return res;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

// Fila nueva con todas las celdas en 0 y next en NULL. La usan gameBoardNew
// y gameBoardCheckAndClearLines, por eso la separe. Devuelve NULL si malloc falla.
static RowNode* nuevaFilaVacia(void) {
    RowNode* fila = malloc(sizeof(RowNode));
    if (fila == NULL) return NULL;
    for (int c = 0; c < GRID_COLS; c++) fila->cells[c] = 0;
    fila->next = NULL;
    return fila;
}

// Va primero porque gameBoardNew la llama si falla a mitad de camino.
// Cada lista se recorre guardando el next ANTES de hacer free, si no
// estariamos leyendo memoria ya liberada.
void gameBoardDelete(GameBoard* board) {
    if (board == NULL) return;

    RowNode* fila = board->first_row;
    while (fila != NULL) {
        RowNode* sig = fila->next;
        free(fila);
        fila = sig;
    }

    PieceQueueNode* pieza = board->queue_head;
    while (pieza != NULL) {
        PieceQueueNode* sig = pieza->next;
        free(pieza);
        pieza = sig;
    }

    ParticleNode* part = board->first_particle;
    while (part != NULL) {
        ParticleNode* sig = part->next;
        free(part);
        part = sig;
    }

    free(board);
}

GameBoard* gameBoardNew() {
    GameBoard* board = malloc(sizeof(GameBoard));
    if (board == NULL) return NULL;

    // dejo todo inicializado antes de pedir las filas, asi si algo falla
    // gameBoardDelete no recorre punteros basura
    board->first_row = NULL;
    board->queue_head = NULL;
    board->queue_tail = NULL;
    board->first_particle = NULL;
    board->current_piece.col = 0;
    board->current_piece.row = 0;
    board->current_piece.type = 0;
    board->current_piece.rotation = 0;
    board->current_piece.active = 0;
    board->score = 0;
    board->tick_timer = 0;

    // las 20 filas se van enganchando al final con un puntero a la ultima,
    // asi first_row queda apuntando a la cima (fila 0)
    RowNode* ultima = NULL;
    for (int i = 0; i < GRID_ROWS; i++) {
        RowNode* fila = nuevaFilaVacia();
        if (fila == NULL) {
            gameBoardDelete(board);   // libera las filas que ya habia pedido
            return NULL;
        }
        if (ultima == NULL) board->first_row = fila;
        else ultima->next = fila;
        ultima = fila;
    }
    return board;
}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    if (board == NULL) return;

    ParticleNode* nodo = malloc(sizeof(ParticleNode));
    if (nodo == NULL) {
        // perder una particula es solo un efecto visual, no vale la pena abortar
        fprintf(stderr, "No hay memoria para una particula, se descarta\n");
        return;
    }

    nodo->x = x;
    nodo->y = y;
    nodo->vel_x = ((rand() % 100) / 10.0f) - 5.0f;
    nodo->vel_y = ((rand() % 50) / -10.0f) - 2.0f;
    nodo->color = color;
    nodo->life_timer = 30 + (rand() % 30);

    // insercion en la cabeza: O(1) y no hace falta guardar el ultimo
    nodo->next = board->first_particle;
    board->first_particle = nodo;
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    if (board == NULL) return 0;

    int borradas = 0;
    int idx = 0;                   // indice de la fila en el tablero ORIGINAL, sirve para ubicar las particulas
    RowNode* prev = NULL;
    RowNode* curr = board->first_row;

    while (curr != NULL) {
        RowNode* sig = curr->next;   // me guardo el siguiente porque curr puede liberarse

        int llena = 1;
        for (int c = 0; c < GRID_COLS; c++) {
            if (curr->cells[c] == 0) {
                llena = 0;
                break;
            }
        }

        // pido la fila de reemplazo ANTES de tocar la lista. Si malloc falla, la fila
        // llena se queda donde esta y el tablero sigue con 20 filas.
        RowNode* nueva = NULL;
        if (llena) {
            nueva = nuevaFilaVacia();
            if (nueva == NULL) fprintf(stderr, "No hay memoria para reemplazar una fila, no se borra\n");
        }

        if (nueva != NULL) {
            // una particula por celda (hay que hacerlo antes del free, necesito los colores)
            for (int c = 0; c < GRID_COLS; c++) {
                gameBoardAddParticle(board,
                                     OFFSET_X + c * CELL_SIZE + CELL_SIZE / 2,
                                     OFFSET_Y + idx * CELL_SIZE + CELL_SIZE / 2,
                                     curr->cells[c]);
            }

            // desenlazo curr. Si es la primera fila no hay prev y se mueve first_row
            if (prev == NULL) board->first_row = sig;
            else prev->next = sig;
            free(curr);

            // la fila nueva va arriba de todo
            nueva->next = board->first_row;
            board->first_row = nueva;

            // si curr era la cabeza, lo que sigue ahora cuelga de la fila nueva,
            // asi que esa pasa a ser el "anterior". Si no, prev no cambia.
            if (prev == NULL) prev = nueva;

            borradas++;
        } else {
            prev = curr;
        }

        curr = sig;
        idx++;
    }
    return borradas;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    if (board == NULL) return;

    PieceQueueNode* nodo = malloc(sizeof(PieceQueueNode));
    if (nodo == NULL) {
        fprintf(stderr, "No hay memoria para encolar la pieza\n");
        return;
    }
    nodo->type = type;
    nodo->next = NULL;

    if (board->queue_tail == NULL) {
        // cola vacia: el nodo es a la vez cabeza y cola
        board->queue_head = nodo;
        board->queue_tail = nodo;
    } else {
        board->queue_tail->next = nodo;
        board->queue_tail = nodo;
    }
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    if (board == NULL || board->queue_head == NULL) return 0;   // vacia: no toco *out_type

    PieceQueueNode* nodo = board->queue_head;
    if (out_type != NULL) *out_type = nodo->type;

    board->queue_head = nodo->next;
    // si era el unico nodo, la cola quedo vacia y tail seguiria apuntando
    // a algo ya liberado. Hay que ponerlo en NULL tambien.
    if (board->queue_head == NULL) board->queue_tail = NULL;

    free(nodo);
    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS =========

static int tests_ok = 0;
static int tests_fail = 0;

static void chequear(int condicion, char* descripcion) {
    if (condicion) {
        tests_ok++;
    } else {
        tests_fail++;
        printf("  FALLO: %s\n", descripcion);
    }
}

// Helpers de tablero, solo para los tests
static int contarFilas(GameBoard* b) {
    int n = 0;
    for (RowNode* f = b->first_row; f != NULL; f = f->next) n++;
    return n;
}

static int contarParticulas(GameBoard* b) {
    int n = 0;
    for (ParticleNode* p = b->first_particle; p != NULL; p = p->next) n++;
    return n;
}

static void llenarFila(GameBoard* b, int idx) {
    RowNode* f = getRowAt(b, idx);
    for (int c = 0; c < GRID_COLS; c++) f->cells[c] = 1;
}

static int filaVacia(RowNode* f) {
    for (int c = 0; c < GRID_COLS; c++) {
        if (f->cells[c] != 0) return 0;
    }
    return 1;
}

static void testStrings(void) {
    printf("Tests de strings...\n");

    // strDuplicate
    char* orig = "hola mundo";
    char* d = strDuplicate("");
    chequear(d != NULL && d[0] == '\0', "strDuplicate de string vacio");
    free(d);

    d = strDuplicate("x");
    chequear(d != NULL && d[0] == 'x' && d[1] == '\0', "strDuplicate de un caracter");
    free(d);

    d = strDuplicate(orig);
    chequear(d != NULL && d != orig, "strDuplicate devuelve memoria nueva");
    chequear(d != NULL && strCompare(d, orig) == 0, "strDuplicate copia el contenido");
    free(d);

    // strCompare
    chequear(strCompare("", "") == 0, "strCompare vacios");
    chequear(strCompare("a", "a") == 0, "strCompare un caracter igual");
    chequear(strCompare("a", "b") == 1, "strCompare a < b da 1");
    chequear(strCompare("b", "a") == -1, "strCompare b > a da -1");
    chequear(strCompare("casa", "casas") == 1, "strCompare prefijo es menor");
    chequear(strCompare("casas", "casa") == -1, "strCompare prefijo en el otro orden");
    chequear(strCompare("", "a") == 1, "strCompare vacio contra no vacio");

    // strConcatenate (los argumentos salen de strDuplicate porque se liberan adentro)
    char* a = strDuplicate("");
    char* b = strDuplicate("abc");
    char* r = strConcatenate(a, b);
    chequear(r != NULL && strCompare(r, "abc") == 0, "strConcatenate \"\" + \"abc\"");
    free(r);

    a = strDuplicate("hola ");
    b = strDuplicate("mundo");
    r = strConcatenate(a, b);
    chequear(r != NULL && strCompare(r, "hola mundo") == 0, "strConcatenate \"hola \" + \"mundo\"");
    free(r);
}

static void testCola(void) {
    printf("Tests de la cola de piezas...\n");
    GameBoard* b = gameBoardNew();
    chequear(b != NULL, "gameBoardNew devuelve un tablero");
    if (b == NULL) return;

    int orden[5] = {3, 1, 4, 1, 5};
    for (int i = 0; i < 5; i++) gameBoardEnqueuePiece(b, orden[i]);

    int out = -99;
    int fifo_ok = 1;
    for (int i = 0; i < 5; i++) {
        int ret = gameBoardDequeuePiece(b, &out);
        if (ret != 1 || out != orden[i]) fifo_ok = 0;
    }
    chequear(fifo_ok, "5 pops salen en el mismo orden en que entraron");

    out = -99;
    chequear(gameBoardDequeuePiece(b, &out) == 0, "el sexto pop devuelve 0");
    chequear(out == -99, "el pop con cola vacia no modifica out_type");
    chequear(b->queue_head == NULL, "queue_head en NULL con la cola vacia");
    chequear(b->queue_tail == NULL, "queue_tail en NULL con la cola vacia");

    // despues de vaciarla tiene que seguir funcionando
    gameBoardEnqueuePiece(b, 6);
    chequear(gameBoardDequeuePiece(b, &out) == 1 && out == 6, "push despues de vaciar y pop");
    chequear(b->queue_head == NULL && b->queue_tail == NULL, "cola vacia otra vez");

    gameBoardDelete(b);
}

static void testLineas(void) {
    printf("Tests de lineas del tablero...\n");

    // una linea (la de abajo)
    GameBoard* b = gameBoardNew();
    chequear(b != NULL, "gameBoardNew para test de lineas");
    if (b == NULL) return;
    chequear(contarFilas(b) == GRID_ROWS, "el tablero arranca con 20 filas");

    RowNode* vieja_cima = b->first_row;
    llenarFila(b, 19);
    chequear(gameBoardCheckAndClearLines(b) == 1, "borrar la fila 19 devuelve 1");
    chequear(b->first_row != vieja_cima, "first_row es una fila nueva");
    chequear(filaVacia(b->first_row), "la fila nueva esta vacia");
    chequear(contarFilas(b) == GRID_ROWS, "siguen las 20 filas");
    chequear(filaVacia(getRowAt(b, 19)), "la fila 19 ya no esta llena");
    chequear(contarParticulas(b) == GRID_COLS, "una particula por celda de la linea");
    gameBoardDelete(b);

    // dos lineas seguidas, empezando por la primera de la lista
    b = gameBoardNew();
    if (b == NULL) { chequear(0, "gameBoardNew"); return; }
    llenarFila(b, 0);
    llenarFila(b, 1);
    chequear(gameBoardCheckAndClearLines(b) == 2, "borrar filas 0 y 1 devuelve 2");
    chequear(contarFilas(b) == GRID_ROWS, "siguen 20 filas tras borrar 0 y 1");
    chequear(filaVacia(b->first_row), "la cima esta vacia");
    chequear(contarParticulas(b) == 2 * GRID_COLS, "20 particulas, sin tope fijo");
    gameBoardDelete(b);

    // lineas separadas, una en cada punta
    b = gameBoardNew();
    if (b == NULL) { chequear(0, "gameBoardNew"); return; }
    llenarFila(b, 0);
    llenarFila(b, 19);
    chequear(gameBoardCheckAndClearLines(b) == 2, "borrar filas 0 y 19 devuelve 2");
    chequear(contarFilas(b) == GRID_ROWS, "siguen 20 filas tras borrar 0 y 19");
    gameBoardDelete(b);

    // una fila a medio llenar no se borra
    b = gameBoardNew();
    if (b == NULL) { chequear(0, "gameBoardNew"); return; }
    llenarFila(b, 10);
    getRowAt(b, 10)->cells[4] = 0;
    chequear(gameBoardCheckAndClearLines(b) == 0, "fila incompleta no se borra");
    chequear(contarFilas(b) == GRID_ROWS && contarParticulas(b) == 0, "no cambia nada");
    gameBoardDelete(b);

    gameBoardDelete(NULL);   // tiene que tolerar NULL
}

void correrTests() {
    testStrings();
    testCola();
    testLineas();
    printf("Resultado: %d ok, %d fallidos\n", tests_ok, tests_fail);
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return tests_fail ? 1 : 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("No se pudo crear el tablero (sin memoria?).\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
