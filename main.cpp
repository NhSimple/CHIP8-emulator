#include <iostream>
#include <cstdint>
#include <array>
#include <string>
#include <stack>
#include <ctime>
#include <chrono>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace sf::Keyboard;

class Chip8
{
public:
    Chip8()
    {
        memory.fill(0);
        V.fill(0);
        display.fill(0);
        keypad.fill(0);
        I = 0;
        DTR = 0;
        STR = 0;
        SP = 0;
        PC = 0x200; // Program start point

        // Load the 4x5 hex font into memory (0x000 to 0x050)
        uint8_t fontSet[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };
        for (int i = 0; i < 80; i++)
            memory[i] = fontSet[i];
    }

    void memoryDataDebug()
    {
        for (int i = 0; i < sizeof(memory); i++)
        {
            if (memory[i] == 0) // ==0 is the same as ==null
            {
                continue;
            }
            string str;
            str = to_string(i);

            cout << str << endl;
        }
    }

    void run()
    {
        std::chrono::time_point<std::chrono::system_clock> last_frame_time = std::chrono::system_clock::now();

        sf::RenderWindow window(sf::VideoMode({640, 320}), "SFML window");

        bool program_running = true;
        while (program_running)
        {
            auto current_time = std::chrono::system_clock::now();
            auto elapsed = current_time - last_frame_time;

            // 1. Run the CPU at its own speed (e.g., 500Hz)
            // You might run 8-10 instructions per "frame"
            execute_instruction();

            // 2. Only tick timers if 1/60th of a second (~16.67ms) has passed
            if (elapsed.count() >= 16.67)
            {
                if (DTR > 0)
                    DTR--;
                if (STR > 0)
                {
                    STR--;
                }
                else
                {
                }
                last_frame_time = current_time;
            }
            debugRender(window);
        }
    }

    void loadRom(std::array<uint8_t, 4096> rom)
    {
        // in a interpreter, bytes: 0-512 are reserved for interpeter ram. Obviously im not using that but we will start at 0x200(512) to emulate that.
        const int start = 0x200;
        for (int i = 0; i < 3584; i++)
        {
            memory[0x200 + i] = rom[i];
        }
    }

    std::array<uint8_t, 4096> loadFromFile(const std::string &filename, Chip8 &system, std::array<uint8_t, 4096> rom)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            std::streampos size = file.tellg();
            std::vector<uint8_t> buffer(size);
            file.seekg(0, std::ios::beg);
            file.read((char *)buffer.data(), size);

            rom.fill(0);
            for (int i = 0; i < size && i < 3584; i++)
            {
                rom[i] = buffer[i];
            }
        }
        return rom;
    }

private:
    std::array<uint8_t, 4096> memory;      // the 4096 bytes of ram.
    uint16_t I;                            // special purpose register
    std::array<uint8_t, 16> V;             // V0–VF general purpose registers.
    uint8_t DTR;                           // delay register
    uint8_t STR;                           // sound register
    uint16_t PC;                           // program counter
    uint8_t SP;                            // Stack pointer (points to top of stack) SO Stack.top()
    stack<uint16_t> Stack;                 // WARNING, cpp has no size limit on stacks. Have to make a method for protection.
    std::array<uint8_t, 16> keypad;        // keypad
    std::array<uint32_t, 64 * 32> display; // standard size of chip-8 displays is 64 by 32 (2048)

    Key *keys_array = new Key[16]{
        Key::X,    // 0
        Key::Num1, // 1
        Key::Num2, // 2
        Key::Num3, // 3
        Key::Q,    // 4
        Key::W,    // 5
        Key::E,    // 6
        Key::A,    // 7
        Key::S,    // 8
        Key::D,    // 9
        Key::Z,    // A
        Key::C,    // B
        Key::Num4, // C
        Key::R,    // D
        Key::F,    // E
        Key::V     // F
    };
    int A = 10;
    int B = 11;
    int C = 12;
    int D = 13;
    int E = 14;
    int F = 15;

    void execute_instruction()
    {
        uint16_t opcode = (memory[PC] << 8) | memory[PC + 1]; // combine the 2 bytes chip-8 opcodes are 16bit

        PC += 2; // fetch cycle complete.

        int x = (opcode & 0x0F00) >> 8;
        int y = (opcode & 0x00F0) >> 4;
        int nnn = (opcode & 0x0FFF);
        int kk = (opcode & 0x00FF);
        int n = (opcode & 0x000F);

        switch (opcode & 0xF000)
        {
        case 0x0000:
            if (opcode == 0x00E0)
                cls();
            else if (opcode == 0x00EE)
                ret();
            else
                cout << "unknown opcode" << opcode << endl;
            break;

        case 0x1000:
            jp(nnn);
            break;
        case 0x2000:
            call(nnn);
            break;
        case 0x3000:
            se(x, kk);
            break;
        case 0x4000:
            sne(x, kk);
            break;
        case 0x5000:
            seRegister(x, y);
            break;
        case 0x6000:
            ld(x, kk);
            break;
        case 0x7000:
            add(x, kk);
            break;
        case 0x8000:
            switch (n)
            {
            case 0x0:
                ldRegister(x, y);
                break;
            case 0x1:
                orBwise(x, y);
                break;
            case 0x2:
                andBwise(x, y);
                break;
            case 0x3:
                xorBwise(x, y);
                break;
            case 0x4:
                addRegister(x, y);
                break;
            case 0x5:
                subRegister(x, y);
                break;
            case 0x6:
                shr(x);
                break;
            case 0x7:
                subn(x, y);
                break;
            case 0xE:
                shl(x);
                break;
            }
            break;
        case 0x9000:
            sneRegister(x, y);
            break;
        case 0xA000:
            si(nnn);
            break;
        case 0xB000:
            jpRegister(nnn);
            break;
        case 0xC000:
            rnd(x, kk);
            break;
        case 0xD000:
            draw(x, y, n);
            break;
        case 0xE000:
            if (kk == 0x9E)
                kbrdSKP(x);
            else if (kk == 0xA1)
                kbrdSKNP(x);
            break;
        case 0xF000:
            switch (kk)
            {
            case 0x07:
                ldD(x);
                break;
            case 0x0A:
                ldP(x);
                break;
            case 0x15:
                setDRegister(x);
                break;
            case 0x18:
                setSRegister(x);
                break;
            case 0x1E:
                addIRegister(x);
                break;
            case 0x29:
                ldF(x);
                break;
            case 0x33:
                bcd(x);
                break;
            case 0x55:
                ldIRegisterMultiple(x);
                break;
            case 0x65:
                rdIRegisterMultiple(x);
                break;
            }
            break;

        default:
            break;
        }
    }

    void cls()
    {
        display.fill(0);
    }

    void ret()
    {
        PC = Stack.top();
        Stack.pop();
    }

    void jp(int addr)
    {
        PC = addr;
    }

    void call(int addr)
    {
        // we dont update SP because we use stack so its redundant, stack.top()
        Stack.push(PC);
        PC = addr;
    }

    void se(int Vx, int value)
    {

        if (V[Vx] == value)
        {
            PC += 2;
        }
    }

    void sne(int Vx, int value)
    {

        if (V[Vx] != value)
        {
            PC += 2;
        }
    }

    void seRegister(int Vx, int Vy) // compares the vals in two registers
    {
        if (V[Vx] == V[Vy])
        {
            PC += 2;
        }
    }

    void ld(int Vx, int value)
    {
        V[Vx] = value;
    }

    void add(int Vx, int value)
    {
        V[Vx] += value;
    }

    void ldRegister(int Vx, int Vy) // copies the value of register y to register x
    {
        V[Vx] = V[Vy];
    }

    void orBwise(int Vx, int Vy) // bitwise or operation result stored in Vx
    {
        V[Vx] = V[Vx] | V[Vy];
    }

    void andBwise(int Vx, int Vy) // bitwise and operation result stored in Vx
    {
        V[Vx] = V[Vx] & V[Vy];
    }

    void xorBwise(int Vx, int Vy) // bitwise XOR operation result stored in VX
    {
        V[Vx] = V[Vx] ^ V[Vy];
    }

    void addRegister(int Vx, int Vy)
    {
        uint16_t sum = V[Vx] + V[Vy];

        if (sum > 255)
        {
            V[F] = 1; // essentially, carry around back for binary addition
        }
        else
        {
            V[F] = 0; // no carry
        }
        V[Vx] = (uint8_t)(sum & 0xFF);
    }

    void subRegister(int Vx, int Vy)
    {
        V[F] = (V[Vx] >= V[Vy]) ? 1 : 0; // sets the flag
        V[Vx] -= V[Vy];
    }

    void shr(int Vx)
    {
        V[F] = (V[Vx] & 0x1);

        // 2. Perform the actual shift
        V[Vx] >>= 1;
    }

    void subn(int Vx, int Vy)
    {
        V[F] = (V[Vy] >= V[Vx]) ? 1 : 0;
        V[Vx] = V[Vy] - V[Vx];
    }

    void shl(int Vx)
    {
        V[F] = (V[Vx] & 0x80) >> 7;
        V[Vx] <<= 1;
    }

    void sneRegister(int Vx, int Vy)
    {
        if (V[Vx] != V[Vy])
        {
            PC += 2;
        }
    }

    void si(int addr)
    {
        I = addr;
    }

    void jpRegister(int addr) // sets PC to addr +Register 0
    {
        PC = V[0] + addr;
    }

    void rnd(int Vx, int value)
    {
        uint8_t randomByte = rand() % 256;
        V[Vx] = randomByte & value;
    }

    void draw(int Vx, int Vy, int height)
    {
        // start coords from reg
        uint8_t xStart = V[Vx] % 64;
        uint8_t yStart = V[Vy] % 32;

        V[F] = 0; // reset collision flag

        // go thru each row
        for (int row = 0; row < height; row++)
        {
            uint8_t spriteByte = memory[I + row];

            // loop through 8 bits in row
            for (int col = 0; col < 8; col++)
            {

                if ((spriteByte & (0x80 >> col)) != 0)
                {

                    // calc pos 1D display array
                    int x = (xStart + col) % 64;
                    int y = (yStart + row) % 32;
                    int index = x + (y * 64);

                    // collision Check
                    if (display[index] == 1)
                    {
                        V[F] = 1;
                    }

                    //  XOR the pixel
                    display[index] ^= 1;
                }
            }
        }
    }

    void kbrdSKP(int Vx)
    {
        if (keypad[V[Vx]])
        {
            PC += 2;
        }
    }

    void kbrdSKNP(int Vx)
    {
        if (!keypad[V[Vx]])
        {
            PC += 2;
        }
    }

    void ldD(int Vx) // loads delay timer to vx
    {
        V[Vx] = DTR;
    }

    void ldP(int Vx) // halts all program execution until ANY key is pressed. KEY pressed stored in specified register.
    {
        bool anyPressed = false;
        for (int i = 0; i < sizeof(keypad); i++)
        {
            if (keypad[i] != 0)
            {
                V[Vx] = i;
                anyPressed = true;
                break;
            }
        }

        // If no key was found this cycle, move PC back
        // so we execute this SAME instruction again next time.
        if (!anyPressed)
        {
            PC -= 2;
        }
    }

    void setDRegister(int Vx)
    {
        DTR = V[Vx];
    }

    void setSRegister(int Vx)
    {
        STR = V[Vx];
    }

    void addIRegister(int Vx)
    {
        I = I + V[Vx];
    }

    void ldF(int Vx)
    {
        // V[Vx] contains the digit (0-15)
        // We point I to the start of that digit's sprite in memory
        I = V[Vx] * 5;
    }

    void bcd(int Vx)
    {
        uint8_t value = V[Vx];

        memory[I] = value / 100;           // Get the hundreds
        memory[I + 1] = (value / 10) % 10; // Get the tens
        memory[I + 2] = value % 10;        // Get the ones
    }

    void ldIRegisterMultiple(int Vx)
    {
        for (int i = 0; i <= Vx; i++)
        {
            memory[I + i] = V[i];
        }
    }

    void rdIRegisterMultiple(int Vx)
    {
        for (int i = 0; i <= Vx; i++)
        {
            V[i] = memory[I + i];
        }
    }

    void debugRender(sf::RenderWindow &window)
    {
        window.clear();
        sf::VertexArray vertexArray(sf::PrimitiveType::Points);
        sf::View view(sf::FloatRect({0.f, 0.f}, {64.f, 32.f}));

        while (const std::optional event = window.pollEvent())
        {
            // Window closed or escape key pressed: exit
            if (event->is<sf::Event::Closed>() ||
                (event->is<sf::Event::KeyPressed>() &&
                 event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape))
            {
                cout << "Bye! :)" << endl;
                exit(0);
            }
        }

        window.setView(view);

        sf::Vertex point;
        for (int y = 0; y < 32; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                // Print a '#' for ON and a '.' for OFF
                if (display[x + (y * 64)] != 0)
                {
                    point.position = sf::Vector2f(x, y);
                    point.color = sf::Color::White;
                    vertexArray.append(point);
                }
                else
                    cout << " ";
            }
            cout << endl;
        }
        for (int i = 0; i < 16; i++)
        {
            if (isKeyPressed(keys_array[i]))
            {
                keypad[i] = 1;
            }
            else
                keypad[i] = 0;
        }
        window.draw(vertexArray);
        window.display();
    }
};

int main(int argc, char *argv[])
{
    std::cout << "CHIP-8 Emulator Started!" << std::endl;
    Chip8 system = Chip8();
    std::array<uint8_t, 4096> rom;

    rom.fill(0);

    if (argc > 1)
    {
        rom = system.loadFromFile(argv[1], system, rom);
        system.loadRom(rom);
        system.run();
    }
    else
    {

        // 1. F029 -> Point I to the sprite for '0' (located in your font set)
        rom[0] = 0xF0;
        rom[1] = 0x29;

        // 2. 600A -> Set V0 to 10 (X coordinate)
        rom[2] = 0x60;
        rom[3] = 0x0A;

        // 3. 610A -> Set V1 to 10 (Y coordinate)
        rom[4] = 0x61;
        rom[5] = 0x0A;

        // 4. D015 -> Draw 5 rows of the sprite at (V0, V1)
        rom[6] = 0xD0;
        rom[7] = 0x15;

        // 5. 1208 -> Infinite loop so it doesn't crash
        rom[8] = 0x12;
        rom[9] = 0x08;

        system.loadRom(rom);
        system.run();
    }
    return 0;
}
