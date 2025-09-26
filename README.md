# gameOfLife.c

This is a **Conway's Game of Life** implementation written purely in **C**, designed to run on **Windows, Linux, and Unix** system's `Terminal`.

![GameOfLife-Rocket](https://github.com/user-attachments/assets/d674c48c-5c8b-4f57-be84-2322e4a806d5)
![GameOfLife-HoneyBee](https://github.com/user-attachments/assets/828e1801-0355-4b97-9088-029b229e6b0b)

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
| D         | Advance to the next iteration   |
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
