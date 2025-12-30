# CHIP-8 Emulator (C++ & SFML)

A high-performance CHIP-8 interpreter built from scratch in C++ using the SFML graphics library. This project emulates the 1970s virtual game console, handling opcode execution, timers, memory management, and XOR-based graphics.

##  Features
- **Full Instruction Set:** Implements all 34 original CHIP-8 opcodes.
- **Graphic Display:** 64x32 monochrome display scaled 10x for modern monitors.
- **Sound & Delay Timers:** Accurate 60Hz hardware timers.
- **Keypad Support:** Mapped 4x4 hex keypad to QWERTY keyboard.
- **ROM Loading:** Load any `.ch8` file via command line arguments.

## Controls
The CHIP-8 used a hexadecimal 4x4 keypad. This emulator maps them to the following PC keys:

| CHIP-8 | PC Key | CHIP-8 | PC Key |
|:---:|:---:|:---:|:---:|
| 1 | 1 | 2 | 2 |
| 3 | 3 | C | 4 |
| 4 | Q | 5 | W |
| 6 | E | D | R |
| 7 | A | 8 | S |
| 9 | D | E | F |
| A | Z | 0 | X |
| B | C | F | V |

##  Building and Running

### Prerequisites
Make sure you have SFML installed on your system:
- **Arch Linux:** `sudo pacman -S sfml`
- **Ubuntu/Debian:** `sudo apt install libsfml-dev`
- **macOS:** `brew install sfml`

### Compilation
1. Create a build directory:
   ```bash
   mkdir build && cd build```
2. run cmake and build:
    ```cmake ..```
3. run!:
 ```./chip8emu path/to/rom.ch8```