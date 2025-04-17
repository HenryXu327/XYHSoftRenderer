#pragma once
#include <cassert>
#include <cmath>

class Color
{
public:
    float r, g, b, a;

public:
    Color(float r, float g, float b, float a);
    Color(float r, float g, float b);
    Color(const Color& color);
    Color();

    ~Color();

    Color operator+(const Color& color) const;
    Color operator+(const float& c)
    {
        return Color(r + c, g + c, b + c, a);
    }
    Color operator-(const Color& color) const;
    Color operator*(const float& scalar) const;
    Color operator*(const Color& color) const;
    Color operator/(const float& scalar) const;
    
    static Color lerp(const Color& color1, const Color& color2, float weight);
    static Color white;
    static Color black;
    static Color red;
    static Color green;
    static Color blue;
    static Color yellow;
    static Color cyan;
};

