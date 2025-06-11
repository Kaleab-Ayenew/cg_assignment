# OpenGL Tic-Tac-Toe

This is a simple, fully playable Tic-Tac-Toe game built with C++ and modern OpenGL. It serves as a practical example of 2D rendering, text rendering, and basic UI interaction in an OpenGL application.

## Features

- Interactive, clickable 3x3 game board.
- Displays whose turn it is ("Player X" or "Player O").
- Automatically detects and announces the winner or a draw.
- Keeps track of the score for both players.
- A "Restart Game" button to start a new round at any time.

## How It Works

The application is contained within a single `main.cpp` file and uses a standard game loop.

### Game Logic

- A `3x3` integer array `board[3][3]` represents the game state (0 for empty, 1 for X, 2 for O).
- `currentPlayer` tracks whose turn it is.
- After each move, `checkGameStatus()` is called to check for a win (rows, columns, diagonals) or a draw.
- Mouse clicks are handled in `mouse_button_callback`, which determines if a click was on the board or the "Restart" button.

### Rendering

The entire scene is rendered using a 2D orthographic projection. The coordinate system has its origin `(0,0)` at the **bottom-left** corner of the window, which is a standard convention for OpenGL.

- **Primitives (Board, X, O, Button):**
  - The grid lines, X's, O's (circles), and the button are drawn using simple geometric shapes.
  - Functions like `drawRect`, `drawLine`, and `drawCircle` buffer vertex data to a shared VAO/VBO and draw them using a simple shader program (`primitiveShaderProgram`).

- **Text Rendering:**
  - The font (`Roboto-Regular.ttf`) is loaded using the `stb_truetype` library.
  - `initTextRendering` pre-renders each character of the ASCII set into a separate OpenGL texture (a "glyph atlas").
  - The `RenderText` function then draws a string by iterating through its characters, binding the corresponding glyph texture, and drawing it on a 2D quad.

## Dependencies

The project relies on the following libraries, which are fetched automatically by CMake:

- **GLFW:** For creating windows and handling input.
- **GLAD:** To load modern OpenGL function pointers.
- **GLM (OpenGL Mathematics):** For vector and matrix operations.
- **stb_truetype:** For loading and rendering fonts.

## How to Build and Run

This project uses CMake for building.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Kaleab-Ayenew/cg_assignment.git
    cd cg_assignment
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake and Make:**
    ```bash
    cmake ..
    make
    ```

4.  **Run the application:**
    ```bash
    ./tictactoe
    ``` 
