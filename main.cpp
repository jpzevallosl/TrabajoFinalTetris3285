#include <GL/freeglut.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 22;
const int CELL_SIZE = 25;
const int PREVIEW_SIZE = 5;

int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};
int score = 0;

int tetrominoes[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444},
    {0x0660, 0x0660, 0x0660, 0x0660},
    {0x0720, 0x2320, 0x2700, 0x2620},
    {0x0360, 0x2310, 0x0360, 0x2310},
    {0x0630, 0x1320, 0x0630, 0x1320},
    {0x0740, 0x2230, 0x1700, 0x3220},
    {0x0710, 0x3220, 0x0470, 0x2260}
};

float colors[7][3] = {
    {1.0f, 1.0f, 0.0f}, // Amarillo
    {1.0f, 0.0f, 0.0f}, // Rojo
    {0.0f, 1.0f, 0.0f}, // Verde
    {0.0f, 0.0f, 1.0f}, // Azul
    {1.0f, 0.5f, 0.0f}, // Naranja
    {0.0f, 1.0f, 1.0f}, // Cian
    {0.5f, 0.0f, 0.5f}  // Púpura
};

int currentTetromino, currentRotation, currentX, currentY;
int nextTetromino;

bool fastDrop = false;
int baseDropSpeed = 500;

void drawCell(int x, int y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + CELL_SIZE, y);
    glVertex2i(x + CELL_SIZE, y + CELL_SIZE);
    glVertex2i(x, y + CELL_SIZE);
    glEnd();
}

void drawSeparatorLine() {
    glColor3f(1.0f, 1.0f, 1.0f); // Color Blanco
    glBegin(GL_LINES);
    glVertex2i(BOARD_WIDTH * CELL_SIZE, 0);
    glVertex2i(BOARD_WIDTH * CELL_SIZE, BOARD_HEIGHT * CELL_SIZE);
    glEnd();

    glColor3f(1.0f, 0.0f, 0.0f); // Color Rojo
    glBegin(GL_LINES);
    glVertex2i(BOARD_WIDTH * CELL_SIZE + 1, 0);
    glVertex2i(BOARD_WIDTH * CELL_SIZE + 1, BOARD_HEIGHT * CELL_SIZE);
    glEnd();
}

bool isColliding(int tetromino, int rotation, int x, int y) {
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            if (tetrominoes[tetromino][rotation] & (1 << (px + py * 4))) {
                int boardX = x + px;
                int boardY = y + py;

                if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT) {
                    return true;
                }

                if (boardY >= 0 && board[boardY][boardX]) {
                    return true;
                }
            }
        }
    }
    return false;
}

void spawnTetromino() {
    currentTetromino = nextTetromino;
    currentRotation = rand() % 4;
    currentX = BOARD_WIDTH / 2;
    currentY = 0;

    nextTetromino = rand() % 7;

    if (isColliding(currentTetromino, currentRotation, currentX, currentY)) {
        // Fin del juego
        int level = score / 1000 + 1;
        std::string message = "Juego terminado, llegaste hasta el nivel " + std::to_string(level);
#ifdef _WIN32
        MessageBox(NULL, message.c_str(), "Game Over", MB_OK);
#endif
        exit(0);
    }
}

void removeLine(int line) {
    for (int y = line; y > 0; y--) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[y][x] = board[y - 1][x];
        }
    }
    score += 100; // Aumentar puntuación
}

void checkLines() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        bool isLineComplete = true;

        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                isLineComplete = false;
                break;
            }
        }

        if (isLineComplete) {
            removeLine(y);
            y--; // Verifique la misma línea nuevamente a medida que se movieron hacia abajo
        }
    }
}

void drawBoard() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int value = board[y][x];
            if (value) {
                float r = colors[value - 1][0];
                float g = colors[value - 1][1];
                float b = colors[value - 1][2];
                drawCell(x * CELL_SIZE, y * CELL_SIZE, r, g, b);
            }
        }
    }
}

void drawPreview() {
    int startX = BOARD_WIDTH * CELL_SIZE + CELL_SIZE;
    int startY = CELL_SIZE;

    float r = colors[nextTetromino][0];
    float g = colors[nextTetromino][1];
    float b = colors[nextTetromino][2];

    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            if (tetrominoes[nextTetromino][0] & (1 << (px + py * 4))) {
                drawCell(startX + px * CELL_SIZE, startY + py * CELL_SIZE, r, g, b);
            }
        }
    }

    // Separador de línea
    glColor3f(1.0f, 0.0f, 0.0f); // Color rojo
    glBegin(GL_LINES);
    glVertex2i(startX, startY + 9 * CELL_SIZE);
    glVertex2i(startX + 4 * CELL_SIZE, startY + 9 * CELL_SIZE);
    glEnd();

    // Puntuación
    glColor3f(1.0, 1.0, 1.0); // Texto blanco
    glRasterPos2i(startX, startY + 19.5 * CELL_SIZE); // Posición de la puntuación
    std::string scoreText = "Score: " + std::to_string(score);
    for (char c : scoreText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Mostrar nivel
    int level = score / 1000 + 1;
    glRasterPos2i(startX, startY + 20.5 * CELL_SIZE); // Posición del nivel
    std::string levelText = "Level: " + std::to_string(level);
    for (char c : levelText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBoard();

    // Dibujar Tetromino actual con su color
    float r = colors[currentTetromino][0];
    float g = colors[currentTetromino][1];
    float b = colors[currentTetromino][2];
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            if (tetrominoes[currentTetromino][currentRotation] & (1 << (px + py * 4))) {
                drawCell((currentX + px) * CELL_SIZE, (currentY + py) * CELL_SIZE, r, g, b);
            }
        }
    }

    drawPreview();

    drawSeparatorLine();

    glutSwapBuffers();
}

void onTimer(int value) {
    int dropSpeed = fastDrop ? 50 : baseDropSpeed - (score / 1000) * 50;
    dropSpeed = std::max(dropSpeed, 100); // Velocidad no meno 100ms

    if (isColliding(currentTetromino, currentRotation, currentX, currentY + 1)) {
        // Tetrominos en la pantalla
        for (int px = 0; px < 4; px++) {
            for (int py = 0; py < 4; py++) {
                if (tetrominoes[currentTetromino][currentRotation] & (1 << (px + py * 4))) {
                    int boardX = currentX + px;
                    int boardY = currentY + py;
                    if (boardY >= 0) {
                        board[boardY][boardX] = currentTetromino + 1;
                    }
                }
            }
        }
        checkLines();
        spawnTetromino();
    } else {
        currentY++;
    }

    glutTimerFunc(dropSpeed, onTimer, 0);
    glutPostRedisplay();
}

void onKeyPress(unsigned char key, int x, int y) {
    if (key == 'a' || key == 'A') {
        if (!isColliding(currentTetromino, currentRotation, currentX - 1, currentY)) {
            currentX--;
        }
    } else if (key == 'd' || key == 'D') {
        if (!isColliding(currentTetromino, currentRotation, currentX + 1, currentY)) {
            currentX++;
        }
    } else if (key == 's' || key == 'S') {
        fastDrop = true;
    } else if (key == 'w' || key == 'W') {
        int newRotation = (currentRotation + 1) % 4;
        if (!isColliding(currentTetromino, newRotation, currentX, currentY)) {
            currentRotation = newRotation;
        }
    }
}

void onKeyRelease(unsigned char key, int x, int y) {
    if (key == 's' || key == 'S') {
        fastDrop = false;
    }
}

int main(int argc, char **argv) {
    srand(time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize((BOARD_WIDTH + 6) * CELL_SIZE, BOARD_HEIGHT * CELL_SIZE); // Aumento del ancho de la ventana en 1
    glutCreateWindow("Tetris");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (BOARD_WIDTH + 6) * CELL_SIZE, BOARD_HEIGHT * CELL_SIZE, 0, -1.0, 1.0); // Aumento del ancho de proyección en 1

    spawnTetromino();

    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyPress);
    glutKeyboardUpFunc(onKeyRelease);
    glutTimerFunc(baseDropSpeed, onTimer, 0);

    glutMainLoop();

    return 0;
}
