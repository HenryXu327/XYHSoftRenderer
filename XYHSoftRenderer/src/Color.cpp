#include "../include/Color.h"

Color Color::white = Color(1.0f, 1.0f, 1.0f, 1.0f);
Color Color::black = Color(0.0f, 0.0f, 0.0f, 1.0f);
Color Color::red = Color(1.0f, 0.0f, 0.0f, 1.0f);
Color Color::green = Color(0.0f, 1.0f, 0.0f, 1.0f);
Color Color::blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
Color Color::yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
Color Color::cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);

Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

Color::Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}

Color::Color(const Color& color) : r(color.r), g(color.g), b(color.b), a(color.a) {}

Color::Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}

Color::~Color() {}

Color Color::operator+(const Color& color) const
{
    return Color(r + color.r, g + color.g, b + color.b, a + color.a);
}

Color Color::operator-(const Color& color) const 
{
    return Color(r - color.r, g - color.g, b - color.b, a - color.a);
}

Color Color::operator*(const float& scalar) const
{
    return Color(r * scalar, g * scalar, b * scalar, a * scalar);
}

Color Color::operator*(const Color& color) const
{
    return Color(r * color.r, g * color.g, b * color.b, a * color.a);
}

Color Color::operator/(const float& scalar) const
{
    assert(scalar != 0.0f);
    float invScalar = 1.0f / scalar;
    return Color(r * invScalar, g * invScalar, b * invScalar, a * invScalar);
}

Color Color::lerp(const Color& color1, const Color& color2, float weight)
{
    return color1 + (color2 - color1) * weight;
}
