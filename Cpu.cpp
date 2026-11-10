#include "VoleMachine.h"
 
// Fetch -> Decode -> Execute (one full instruction)
void CPU::step(Memory& memory, Screen& screen) {
    if (halted) return;
 
    // Instruction is 2 bytes: if PC = 255 there is no room for the 2nd byte
    if (pc == 255) {
        halted = true;
        return;
    }
 
    // Fetch
    ir = static_cast<Word>((memory.read(pc) << 8) | memory.read(pc + 1));
    pc = static_cast<Byte>(pc + 2);  // Jump may overwrite it during execute
 
    // Decode
    if (!InstructionFactory::isValid(ir)) {
        halted = true;
        return;
    }
    std::unique_ptr<Instruction> inst = InstructionFactory::create(ir);
 
    // Execute
    inst->execute(*this, memory, screen);
}
 
void CPU::runStep(Memory& memory, Screen& screen) {
    if (!halted) step(memory, screen);
}
 
Byte CPU::getPC() { return pc; }
void CPU::setPC(Byte address) { pc = address; }
Word CPU::getIR() { return ir; }
bool CPU::isHalted() { return halted; }
void CPU::halt() { halted = true; }
RegisterFile& CPU::getRegisters() { return registers; }
 


