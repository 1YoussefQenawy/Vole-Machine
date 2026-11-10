
#include "VoleMachine.h"


Instruction::Instruction(int opcode, int r, int s, int t, int xy)
    : opcode(opcode), r(r), s(s), t(t), xy(xy) {}

/* * * * * * * * * * * * * * * * * * * * * * */


void LoadFromMemory::execute(CPU& cpu, Memory& memory, Screen& screen) {
    cpu.getRegisters().set(r, memory.read(xy));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void LoadImmediate::execute(CPU& cpu, Memory& memory, Screen& screen) {
    cpu.getRegisters().set(r, static_cast<Byte>(xy));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void StoreToMemory::execute(CPU& cpu, Memory& memory, Screen& screen) {
    memory.write(xy, cpu.getRegisters().get(r));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void StoreToScreen::execute(CPU& cpu, Memory& memory, Screen& screen) {
    screen.write(cpu.getRegisters().get(r));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void Move::execute(CPU& cpu, Memory& memory, Screen& screen) {
    cpu.getRegisters().set(t, cpu.getRegisters().get(s));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void AddInteger::execute(CPU& cpu, Memory& memory, Screen& screen) {
    Byte valueS = cpu.getRegisters().get(s);
    Byte valueT = cpu.getRegisters().get(t);

    cpu.getRegisters().set(r, ALU::add(valueS, valueT));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void AddFloat::execute(CPU& cpu, Memory& memory, Screen& screen) {
    Byte valueS = cpu.getRegisters().get(s);
    Byte valueT = cpu.getRegisters().get(t);

    cpu.getRegisters().set(r, ALU::addFloat(valueS, valueT));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void BitOr::execute(CPU& cpu, Memory& memory, Screen& screen) {
    Byte valueS = cpu.getRegisters().get(s);
    Byte valueT = cpu.getRegisters().get(t);

    cpu.getRegisters().set(r, ALU::bitOr(valueS, valueT));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void BitAnd::execute(CPU& cpu, Memory& memory, Screen& screen) {
    Byte valueS = cpu.getRegisters().get(s);
    Byte valueT = cpu.getRegisters().get(t);

    cpu.getRegisters().set(r, ALU::bitAnd(valueS, valueT));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void BitXor::execute(CPU& cpu, Memory& memory, Screen& screen) {
    Byte valueS = cpu.getRegisters().get(s);
    Byte valueT = cpu.getRegisters().get(t);

    cpu.getRegisters().set(r, ALU::bitXor(valueS, valueT));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void Rotate::execute(CPU& cpu, Memory& memory, Screen& screen) {
    Byte value = cpu.getRegisters().get(r);

    cpu.getRegisters().set(r, ALU::rotateRight(value, t));
}

/* * * * * * * * * * * * * * * * * * * * * * */


void Jump::execute(CPU& cpu, Memory& memory, Screen& screen) {
    Byte registerValue = cpu.getRegisters().get(r);
    Byte registerZero = cpu.getRegisters().get(0);

    if (registerValue == registerZero) {
        cpu.setPC(static_cast<Byte>(xy));
    }
}

/* * * * * * * * * * * * * * * * * * * * * * */


void Halt::execute(CPU& cpu, Memory& memory, Screen& screen) {
    cpu.halt();
}

/* * * * * * * * * * * * * * * * * * * * * * */



bool InstructionFactory::isValid(Word instruction) {
    int opcode = (instruction >> 12) & 0xF;
    int r = (instruction >> 8) & 0xF;
    int s = (instruction >> 4) & 0xF;
    int t = instruction & 0xF;

    switch (opcode) {
        case 0x1: // Load from memory
        case 0x2: // Load immediate
        case 0x3: // Store
        case 0x5: // Integer addition
        case 0x6: // Floating point addition
        case 0x7: // OR
        case 0x8: // AND
        case 0x9: // XOR
        case 0xB: // Jump
            return true;

        case 0x4: // Move: 40RS
            return r == 0;

        case 0xA: // Rotate: AR0X
            return s == 0;

        case 0xC: // Halt: C000 only
            return r == 0 && s == 0 && t == 0;

        default:
            return false;
    }
}


std::unique_ptr<Instruction> InstructionFactory::create(Word instruction) {
    if (!isValid(instruction)) {
        return nullptr;
    }

    int opcode = (instruction >> 12) & 0xF;
    int r = (instruction >> 8) & 0xF;
    int s = (instruction >> 4) & 0xF;
    int t = instruction & 0xF;
    int xy = instruction & 0xFF;

    switch (opcode) {
        case 0x1:
            return std::make_unique<LoadFromMemory>(
                opcode, r, s, t, xy
            );

        case 0x2:
            return std::make_unique<LoadImmediate>(
                opcode, r, s, t, xy
            );

        case 0x3:
            if (xy == 0xFF) {
                return std::make_unique<StoreToScreen>(
                    opcode, r, s, t, xy
                );
            }

            return std::make_unique<StoreToMemory>(
                opcode, r, s, t, xy
            );

        case 0x4:
            return std::make_unique<Move>(
                opcode, r, s, t, xy
            );

        case 0x5:
            return std::make_unique<AddInteger>(
                opcode, r, s, t, xy
            );

        case 0x6:
            return std::make_unique<AddFloat>(
                opcode, r, s, t, xy
            );

        case 0x7:
            return std::make_unique<BitOr>(
                opcode, r, s, t, xy
            );

        case 0x8:
            return std::make_unique<BitAnd>(
                opcode, r, s, t, xy
            );

        case 0x9:
            return std::make_unique<BitXor>(
                opcode, r, s, t, xy
            );

        case 0xA:
            return std::make_unique<Rotate>(
                opcode, r, s, t, xy
            );

        case 0xB:
            return std::make_unique<Jump>(
                opcode, r, s, t, xy
            );

        case 0xC:
            return std::make_unique<Halt>(
                opcode, r, s, t, xy
            );

        default:
            return nullptr;
    }
}
