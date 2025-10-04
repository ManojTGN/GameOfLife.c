# gameOfLife.c
This is a **Conway's Game of Life** implementation written purely in **C**, designed to run on **Windows, Linux, and Unix** system's `Terminal`.
<img src="https://github.com/user-attachments/assets/b538f96d-69ff-468d-8f81-107addb0a5db" width="100%" />

## Features
- Works on **Windows, Linux, and Unix** terminals.
- Gradient and with three color visualization for the cells.
- Interactive controls to manipulate the game in real-time.
- Pure C implementation with no external dependencies.
- Supports both UTF8 & NON-UTF8 terminals.

## Controls
| Key       | Action                          |
|-----------|---------------------------------|
| Arrow Keys| Move the cursor                 |
| W         | Increase simulation speed       |
| S         | Decrease simulation speed       |
| A         | Change color mode               |
| D         | Advance to the next iteration   |
| C         | Clear all the cells from sim.   |
| N         | Creates a new game of life sim. |
| Space     | Pause/Resume simulation         |
| Enter     | Create a life at the cursor     |
| Backspace | Delete a life at the cursor     |
| Esc       | Exit the GameOfLife Simulation  |

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
