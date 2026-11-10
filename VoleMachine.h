#ifndef VOLE_MACHINE_H
#define VOLE_MACHINE_H

#include <string>
#include <memory>

using Byte = unsigned char;       // 8 bits
using Word = unsigned short;      // 16 bits

class Memory {
private:
    Byte cells[256]{};
public:
    Byte read(int address);
    void write(int address, Byte value);
    void clear();
    bool isValidStart(int address);
};

class RegisterFile {
private:
    Byte registers[16]{};
public:
    Byte get(int index);
    void set(int index, Byte value);
    void clear();
};

class Screen {
private:
    std::string output;
public:
    void write(Byte value);
    void clear();
    std::string getContent();
};

class ALU {
public:
    static Byte add(Byte a, Byte b);
    static Byte addFloat(Byte a, Byte b);
    static Byte bitOr(Byte a, Byte b);
    static Byte bitAnd(Byte a, Byte b);
    static Byte bitXor(Byte a, Byte b);
    static Byte rotateRight(Byte value, int times);
};

class CPU {
private:
    Byte pc = 0;
    Word ir = 0;
    bool halted = false;
    RegisterFile registers;
public:
    void step(Memory& memory, Screen& screen);
    void runStep(Memory& memory, Screen& screen);
    Byte getPC();
    void setPC(Byte address);
    Word getIR();
    bool isHalted();
    void halt();
    RegisterFile& getRegisters();
};

class Instruction {
protected:
    int opcode, r, s, t, xy;
public:
    Instruction(int opcode, int r, int s, int t, int xy);
    virtual ~Instruction() = default;
    virtual void execute(CPU& cpu, Memory& memory, Screen& screen) = 0;
};

// One class for each supported instruction
class LoadFromMemory : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class LoadImmediate : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class StoreToMemory : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class StoreToScreen : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class Move : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class AddInteger : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class AddFloat : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class BitOr : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class BitAnd : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class BitXor : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class Rotate : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class Jump : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class Halt : public Instruction {
public:
    using Instruction::Instruction;
    void execute(CPU&, Memory&, Screen&) override;
};

class InstructionFactory {
public:
    static std::unique_ptr<Instruction> create(Word instruction);
    static bool isValid(Word instruction);
};

class ProgramLoader {
public:
    bool load(std::string filename, int startAddress, Memory& memory);
};

class VoleMachine {
private:
    CPU cpu;
    Memory memory;
    Screen screen;
    ProgramLoader loader;
public:
    bool loadProgram(std::string filename, int startAddress);
    void runAll();
    void step();
    bool isHalted();
    void reset();

    CPU& getCPU();
    Memory& getMemory();
    Screen& getScreen();
};

class MachineDisplay {
public:
    void showMemory(Memory& memory);
    void showRegisters(RegisterFile& registers);
    void showCPU(CPU& cpu);
    void showScreen(Screen& screen);
    void showAll(VoleMachine& machine);
};

class ConsoleUI {
private:
    VoleMachine machine;
    MachineDisplay display;
public:
    void start();
};

#endif
