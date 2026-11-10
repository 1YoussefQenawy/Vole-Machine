
#include "VoleMachine.h"
#include <cmath>

// Integer Addition
Byte ALU::add(Byte a, Byte b)
{
    return static_cast<Byte>(a + b);
}

// FloatingPoint Addition
// Format S EEE MMMM
// 1 Sign bit, 3 Exponent bits, 4 Mantissa bits
// Bias = 3
Byte ALU::addFloat(Byte a, Byte b)
{
    auto decode = [](Byte x) -> float
    {
        int sign = (x >> 7) & 1;
        int expBits = (x >> 4) & 7;

        if (expBits == 0)
            return 0.0f;

        int exponent = expBits - 3;
        int mantissa = 16 + (x & 15);

        float value = (mantissa / 16.0f) * std::pow(2.0f, exponent);

        return sign ? -value : value;
    };

    float result = decode(a) + decode(b);

    if (result == 0)
        return 0;

    int sign = result < 0;
    result = std::fabs(result);

    int exponent = 0;

    while (result >= 2.0f)
        result /= 2.0f, exponent++;

    while (result < 1.0f)
        result *= 2.0f, exponent--;

    exponent += 3;

    if (exponent <= 0)
        return 0;

    if (exponent >= 8)
        return (sign << 7) | 0x7F;

    int mantissa = std::round((result - 1.0f) * 16);

    if (mantissa >= 16)
    {
        mantissa = 0;
        exponent++;

        if (exponent >= 8)
            return (sign << 7) | 0x7F;
    }

    return (sign << 7) | (exponent << 4) | mantissa;
}

Byte ALU::bitOr(Byte a, Byte b)
{
    return a | b;
}

Byte ALU::bitAnd(Byte a, Byte b)
{
    return a & b;
}

Byte ALU::bitXor(Byte a, Byte b)
{
    return a ^ b;
}

Byte ALU::rotateRight(Byte value, int times)
{
    times %= 8;

    if (times < 0)
        times += 8;

    for (int i = 0; i < times; i++)
    {
        Byte bit = value & 1;
        value >>= 1;

        if (bit)
            value |= 0x80;
    }

    return value;
}
