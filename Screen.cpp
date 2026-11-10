#include "VoleMachine.h"

void Screen::write(Byte value)
{
    output += static_cast<char>(value);
}

void Screen::clear()
{
    output.clear();
}

std::string Screen::getContent()
{
    return output;
}