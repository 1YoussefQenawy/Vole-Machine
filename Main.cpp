#include <iostream>
#include <string>
using namespace std;

//  Memory Class
class Memory {
private:
    string cells[256];

public:
    string read(int address);
    void write(int address, string value);
    void clear();
};

//  CPU Class
class CPU {
private:
    int PC;
    string IR;
    int registers[16];

public:
    void fetchInstruction();
    void decodeInstruction();
    void executeInstruction();
    void reset();
};

//  Instruction Class
class Instruction {
private:
    string opcode;
    string operands;

public:
    Instruction(string code);
    string getOpcode();
    string getOperands();
};

//  ProgramLoader Class
class ProgramLoader {
private:
    string filePath;

public:
    bool loadFromFile(string filePath);
};

//  Screen Class
class Screen {
private:
    string output;

public:
    void display(string value);
    void clear();
};

//  Machine Class
class Machine {
private:
    CPU cpu;
    Memory memory;
    Screen screen;
    ProgramLoader loader;

public:
    void run();
    void step();
    void reset();
};

int main() {
    return 0;
}