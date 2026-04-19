
#pragma once //esto le dice al compilador que incluya el archivo solo una vez, o si no, se puede morir todo

struct Vec2 {
    float x, y;

    Vec2 operator-(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }
};

struct Vec3 {
    float x, y, z;

    Vec3 operator-(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }
};

struct Color {
    int r, g, b;
};

inline float edge_function(const Vec2& a, const Vec2& b) {
    return a.x * b.y - a.y * b.x;
}
