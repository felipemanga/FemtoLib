#pragma once
#include "types.hpp"

struct Point3D : public Point2D {
    f32 z;

    Point3D& operator *= (f32 scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Point3D& operator += (const Point3D &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Point3D& operator *= (const Point3D &other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    Point3D& operator -= (const Point3D &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Point3D operator + (const Point3D &other) {
        Point3D c{*this};
        c.x += other.x;
        c.y += other.y;
        c.z += other.z;
        return c;
    }

    Point3D operator * (const Point3D &other) {
        Point3D c{*this};
        c.x *= other.x;
        c.y *= other.y;
        c.z *= other.z;
        return c;
    }

    Point3D operator - (const Point3D &other) {
        Point3D c{*this};
        c.x -= other.x;
        c.y -= other.y;
        c.z -= other.z;
        return c;
    }

    void rotateXZ(f32 rad){
        f32 cr = cos(rad);
        f32 sr = sin(rad);
        f32 x = cr * this->x - sr * z;
        z = cr * z + sr * this->x;
        this->x = x;
    }

    void rotateYZ(f32 rad){
        f32 cr = cos(rad);
        f32 sr = sin(rad);
        f32 y = cr * this->y - sr * z;
        z = cr * z + sr * this->y;
        this->y = y;
    }
};
