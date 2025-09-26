# gameOfLife.c

This is a **Conway's Game of Life** implementation written purely in **C**, designed to run on **Windows, Linux, and Unix** systems.

@todo: Add Screenshots

## Features
- Works on **Windows, Linux, and Unix** terminals.
- Gradient color visualization for the cells.
- Interactive controls to manipulate the game in real-time.
- Pure C implementation with no external dependencies.

## Controls

| Key       | Action                          |
|-----------|--------------------------------|
| Arrow Keys| Move the cursor                 |
| W         | Increase simulation speed       |
| S         | Decrease simulation speed       |
| D         | Advance to the next iteration  |
| Space     | Pause/Resume simulation         |
| Enter     | Create a life at the cursor     |

## Build and Run

1. Clone the repository:
    ```bash
    git clone https://github.com/ManojTGN/GameOfLife.c.git
    cd GameOfLife.c
    ```

2. Compile the program:
    ```bash
    mkdir build
    cd build

    cmake ..
    cmake --build .
    ```

## License
This project is licensed under the MIT License.  
Feel free to modify and use it as you like!
