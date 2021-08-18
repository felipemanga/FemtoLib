#pragma once
#include "types.hpp"

struct Point3D : public Point2D {
    f32 z;

    Point3D& tweenTo(const Point3D& other, s32 factor) {
        x = ::tweenTo(x, other.x, factor);
        y = ::tweenTo(y, other.y, factor);
        z = ::tweenTo(z, other.z, factor);
        return *this;
    }

    Point3D operator - () {
        Point3D r;
        r.x = -x;
        r.y = -y;
        r.z = -z;
        return r;
    }

    Point2D xz() {
        Point2D r;
        r.x = x;
        r.y = z;
        return r;
    }

    Point2D yz() {
        Point2D r;
        r.x = y;
        r.y = z;
        return r;
    }

    Point2D zx() {
        Point2D r;
        r.x = z;
        r.y = x;
        return r;
    }

    Point2D zy() {
        Point2D r;
        r.x = z;
        r.y = y;
        return r;
    }

    Point2D zz() {
        Point2D r;
        r.x = z;
        r.y = z;
        return r;
    }

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

    Point3D operator * (f32 scalar) const {
        Point3D c{*this};
        c.x *= scalar;
        c.y *= scalar;
        c.z *= scalar;
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


    f32 lengthSquared() {
        return x * x + y * y + z * z;
    }

    f32 length() {
        return sqrt(lengthSquared());
    }

    Point3D normalize() {
        Point3D point = *this;
        f32 len = length();
        if (len) {
            point.x /= len;
            point.y /= len;
            point.z /= len;
        }
        return point;
    }
};
