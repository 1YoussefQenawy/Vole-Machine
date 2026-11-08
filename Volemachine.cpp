#include "VoleMachine.h"
 
bool VoleMachine::loadProgram(std::string filename, int startAddress) {
    if (!loader.load(filename, startAddress, memory)) return false;
    cpu.setPC(static_cast<Byte>(startAddress));  // execution starts at load address
    return true;
}
 
void VoleMachine::step() {
    cpu.runStep(memory, screen);
}
 
void VoleMachine::runAll() {
    const long MAX_STEPS = 1000000;  // protection against infinite loops
    long count = 0;
    while (!cpu.isHalted() && count++ < MAX_STEPS) {
        cpu.runStep(memory, screen);
    }
}
 
bool VoleMachine::isHalted() { return cpu.isHalted(); }
 
void VoleMachine::reset() {
    cpu = CPU();   // resets PC, IR, halted and registers
    memory.clear();
    screen.clear();
}
 
CPU& VoleMachine::getCPU() { return cpu; }
Memory& VoleMachine::getMemory() { return memory; }
Screen& VoleMachine::getScreen() { return screen; }
 