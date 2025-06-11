#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>

// --- Globals & Constants ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Game State
int board[3][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} }; // 0: empty, 1: X, 2: O
int currentPlayer = 1;
bool gameOver = false;
int winner = 0; // 0: none, 1: X, 2: O, 3: Draw
int scoreX = 0;
int scoreO = 0;

// Board drawing properties
const float BOARD_SIZE = 450.0f;
const float CELL_SIZE = BOARD_SIZE / 3.0f;
const float BOARD_X = (SCR_WIDTH - BOARD_SIZE) / 2.0f;
const float BOARD_Y = (SCR_HEIGHT - BOARD_SIZE) / 2.0f - 30.0f; // Move board down for UI
const float LINE_WIDTH = 10.0f;

// Restart button properties
const float BUTTON_X = 300.0f;
const float BUTTON_Y = 530.0f;
const float BUTTON_WIDTH = 200.0f;
const float BUTTON_HEIGHT = 50.0f;

// --- Text Rendering Structs and Globals ---
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    float        Advance;   // Offset to advance to next glyph (in pixel units)
};
std::map<char, Character> Characters;
unsigned int textVAO, textVBO;
unsigned int textShaderProgram;

// --- Primitive Rendering Globals ---
unsigned int primitiveShaderProgram;
unsigned int primitiveVAO, primitiveVBO;

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void resetGame();
void checkGameStatus();
void initPrimitives();
void initTextRendering(const char* fontPath);
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
void drawBoard();
void drawMoves();
void drawUI();
void drawRect(float x, float y, float w, float h, glm::vec3 color);
void drawCircle(float cx, float cy, float r, glm::vec3 color);
void drawLine(float x1, float y1, float x2, float y2, float width, glm::vec3 color);

// --- Shader Sources ---
const char *primitiveVertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)glsl";

const char *primitiveFragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 objectColor;
    void main() {
        FragColor = vec4(objectColor, 1.0);
    }
)glsl";

const char *textVertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
    out vec2 TexCoords;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
)glsl";

const char *textFragmentShaderSource = R"glsl(
    #version 330 core
    in vec2 TexCoords;
    out vec4 FragColor;
    uniform sampler2D text;
    uniform vec3 textColor;
    void main() {
        float alpha = texture(text, TexCoords).r;
        FragColor = vec4(textColor, alpha);
    }
)glsl";


// --- Main Function ---
int main() {
    // --- GLFW & GLAD Initialization ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Tic Tac Toe", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable blending for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Initialize Rendering ---
    initPrimitives();
    initTextRendering("../src/Roboto-Regular.ttf");

    // --- Main Game Loop ---
    while (!glfwWindowShouldClose(window)) {
        // --- Render ---
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawBoard();
        drawMoves();
        drawUI();

        // --- Swap Buffers & Poll Events ---
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    glDeleteVertexArrays(1, &primitiveVAO);
    glDeleteBuffers(1, &primitiveVBO);
    glDeleteProgram(primitiveShaderProgram);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(textShaderProgram);
    glfwTerminate();
    return 0;
}

// --- Game Logic Functions ---
void resetGame() {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            board[i][j] = 0;
        }
    }
    currentPlayer = 1;
    gameOver = false;
    winner = 0;
}

void checkGameStatus() {
    // Check rows and columns
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][1] == board[i][2]) winner = board[i][0];
        if (board[0][i] != 0 && board[0][i] == board[1][i] && board[1][i] == board[2][i]) winner = board[0][i];
    }
    // Check diagonals
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) winner = board[0][0];
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) winner = board[0][2];

    if (winner != 0) {
        gameOver = true;
        if (winner == 1) scoreX++;
        else if (winner == 2) scoreO++;
        return;
    }

    // Check for draw
    bool isDraw = true;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == 0) {
                isDraw = false;
                break;
            }
        }
        if (!isDraw) break;
    }

    if (isDraw) {
        winner = 3; // 3 for Draw
        gameOver = true;
    }
}

// --- Input Handling ---
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = SCR_HEIGHT - ypos; // Invert Y for bottom-left origin

        // Check for restart button click
        if (xpos >= BUTTON_X && xpos <= BUTTON_X + BUTTON_WIDTH &&
            ypos >= BUTTON_Y && ypos <= BUTTON_Y + BUTTON_HEIGHT) {
            resetGame();
            return;
        }

        if (gameOver) return;

        // Check for board click
        if (xpos >= BOARD_X && xpos <= BOARD_X + BOARD_SIZE &&
            ypos >= BOARD_Y && ypos <= BOARD_Y + BOARD_SIZE) {
            
            int col = (xpos - BOARD_X) / CELL_SIZE;
            int row = (ypos - BOARD_Y) / CELL_SIZE;

            if (board[row][col] == 0) {
                board[row][col] = currentPlayer;
                currentPlayer = (currentPlayer == 1) ? 2 : 1;
                checkGameStatus();
            }
        }
    }
}

// --- Drawing Functions ---
void drawBoard() {
    glm::vec3 gridColor(0.8f, 0.8f, 0.8f);
    // Vertical lines
    drawLine(BOARD_X + CELL_SIZE, BOARD_Y, BOARD_X + CELL_SIZE, BOARD_Y + BOARD_SIZE, LINE_WIDTH, gridColor);
    drawLine(BOARD_X + 2 * CELL_SIZE, BOARD_Y, BOARD_X + 2 * CELL_SIZE, BOARD_Y + BOARD_SIZE, LINE_WIDTH, gridColor);
    // Horizontal lines
    drawLine(BOARD_X, BOARD_Y + CELL_SIZE, BOARD_X + BOARD_SIZE, BOARD_Y + CELL_SIZE, LINE_WIDTH, gridColor);
    drawLine(BOARD_X, BOARD_Y + 2 * CELL_SIZE, BOARD_X + BOARD_SIZE, BOARD_Y + 2 * CELL_SIZE, LINE_WIDTH, gridColor);
}

void drawMoves() {
    glm::vec3 xColor(0.9f, 0.2f, 0.2f);
    glm::vec3 oColor(0.2f, 0.5f, 0.9f);
    float padding = 25.0f;
    float symbolLineWidth = 15.0f;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            float cell_x = BOARD_X + j * CELL_SIZE;
            float cell_y = BOARD_Y + i * CELL_SIZE;

            if (board[i][j] == 1) { // Draw X
                drawLine(cell_x + padding, cell_y + padding, cell_x + CELL_SIZE - padding, cell_y + CELL_SIZE - padding, symbolLineWidth, xColor);
                drawLine(cell_x + CELL_SIZE - padding, cell_y + padding, cell_x + padding, cell_y + CELL_SIZE - padding, symbolLineWidth, xColor);
            } else if (board[i][j] == 2) { // Draw O
                drawCircle(cell_x + CELL_SIZE / 2.0f, cell_y + CELL_SIZE / 2.0f, CELL_SIZE / 2.0f - padding, oColor);
            }
        }
    }
}

void drawUI() {
    // Draw restart button
    glm::vec3 buttonColor(0.3f, 0.6f, 0.4f);
    drawRect(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, buttonColor);
    RenderText("Restart Game", BUTTON_X + 25.0f, BUTTON_Y + 18.0f, 0.5f, glm::vec3(1.0f));

    // Draw Scores
    std::string scoreText = "Score: X - " + std::to_string(scoreX) + "  O - " + std::to_string(scoreO);
    RenderText(scoreText, 20.0f, SCR_HEIGHT - 30.0f, 0.5f, glm::vec3(0.9f));

    // Draw Status Message
    std::string statusText;
    glm::vec3 statusColor(1.0f);
    if (gameOver) {
        if (winner == 1) { statusText = "Player X Wins!"; statusColor = glm::vec3(0.9f, 0.2f, 0.2f); }
        else if (winner == 2) { statusText = "Player O Wins!"; statusColor = glm::vec3(0.2f, 0.5f, 0.9f); }
        else { statusText = "It's a Draw!"; statusColor = glm::vec3(0.7f); }
    } else {
        if (currentPlayer == 1) { statusText = "Player X's Turn"; statusColor = glm::vec3(0.9f, 0.2f, 0.2f); }
        else { statusText = "Player O's Turn"; statusColor = glm::vec3(0.2f, 0.5f, 0.9f); }
    }
    RenderText(statusText, 280.0f, 80.0f, 0.7f, statusColor);
}

// --- Primitive Drawing Helpers ---
void initPrimitives() {
    // Shader Program
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &primitiveVertexShaderSource, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &primitiveFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    primitiveShaderProgram = glCreateProgram();
    glAttachShader(primitiveShaderProgram, vertexShader);
    glAttachShader(primitiveShaderProgram, fragmentShader);
    glLinkProgram(primitiveShaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // VAO & VBO
    glGenVertexArrays(1, &primitiveVAO);
    glGenBuffers(1, &primitiveVBO);
    glBindVertexArray(primitiveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 100, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawRect(float x, float y, float w, float h, glm::vec3 color) {
    glUseProgram(primitiveShaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(primitiveShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(primitiveShaderProgram, "objectColor"), 1, &color[0]);
    
    float vertices[] = { x, y, x + w, y, x, y + h, x + w, y + h };
    
    glBindVertexArray(primitiveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void drawLine(float x1, float y1, float x2, float y2, float width, glm::vec3 color) {
    glUseProgram(primitiveShaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(primitiveShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(primitiveShaderProgram, "objectColor"), 1, &color[0]);

    glLineWidth(width);
    float vertices[] = { x1, y1, x2, y2 };
    
    glBindVertexArray(primitiveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

void drawCircle(float cx, float cy, float r, glm::vec3 color) {
    glUseProgram(primitiveShaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(primitiveShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(primitiveShaderProgram, "objectColor"), 1, &color[0]);

    int num_segments = 50;
    std::vector<float> vertices;
    for (int i = 0; i <= num_segments; i++) {
        float angle = i * 2.0f * 3.1415926f / num_segments;
        vertices.push_back(cx + (cos(angle) * r));
        vertices.push_back(cy + (sin(angle) * r));
    }
    
    glBindVertexArray(primitiveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glLineWidth(15.0f);
    glDrawArrays(GL_LINE_STRIP, 0, num_segments + 1);
    glBindVertexArray(0);
}

// --- Text Rendering Helpers ---
void initTextRendering(const char* fontPath) {
    // Shader Program
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, vertexShader);
    glAttachShader(textShaderProgram, fragmentShader);
    glLinkProgram(textShaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load font using stb_truetype
    long size;
    unsigned char* fontBuffer;
    FILE* fontFile = fopen(fontPath, "rb");
    if (!fontFile) {
        std::cerr << "ERROR: FAILED TO LOAD FONT: " << fontPath << std::endl;
        return;
    }
    fseek(fontFile, 0, SEEK_END);
    size = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);
    fontBuffer = (unsigned char*)malloc(size);
    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, fontBuffer, 0)) {
        std::cerr << "ERROR: stbtt_InitFont failed" << std::endl;
    }

    // Create character textures
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
    for (unsigned char c = 0; c < 128; c++) {
        int glyph_w, glyph_h, xoff, yoff;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&info, 0, stbtt_ScaleForPixelHeight(&info, 48.0f), c, &glyph_w, &glyph_h, &xoff, &yoff);

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph_w, glyph_h, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbtt_FreeBitmap(bitmap, 0);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&info, c, &advance, &lsb);
        float scale = stbtt_ScaleForPixelHeight(&info, 48.0f);
        Character character = {
            texture,
            glm::ivec2(glyph_w, glyph_h),
            glm::ivec2(xoff, yoff), // Fix bearing for bottom-left origin
            advance * scale // Store advance in pixel units
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    free(fontBuffer);

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
    glUseProgram(textShaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    for (std::string::const_iterator c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        float ypos = y + (ch.Bearing.y - ch.Size.y) * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        x += ch.Advance * scale; // Remove bitshift, use pixel units
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- GLFW Callbacks ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}