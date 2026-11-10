#include "VoleMachine.h"

Byte RegisterFile::get(int index)
{
    if (index < 0 || index >= 16)
    {
        return 0;
    }

    return registers[index];
}


void RegisterFile::set(int index, Byte value)
{
    if (index < 0 || index >= 16)
    {
        return;
    }

    registers[index] = value;
}
void RegisterFile::clear()
{
    for (int i = 0; i < 16; i++)
    {
        registers[i] = 0;
    }
}