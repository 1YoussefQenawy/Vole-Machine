#include "VoleMachine.h"

Byte Memory::read(int address)
{
    if (address < 0 || address >= 256)
    {
        return 0;
    }

    return cells[address];
}

void Memory::write(int address, Byte value)
{
    if (address < 0 || address >= 256)
    {
        return;
    }

    cells[address] = value;
}

    void Memory::clear()
{
    for (int i = 0; i < 256; i++)
    {
        cells[i] = 0;
    }
}

bool Memory::isValidStart(int address)
{
    return address >= 0 &&
           address <= 254 &&
           address % 2 == 0;
}