#pragma once

#include "types.hpp"

constexpr f32 tweenTo(f32 t, f32 v, s32 speed);

struct Point2D {
    f32 x, y;
    void set(f32 x, f32 y){
        this->x = x;
        this->y = y;
    }

    Point2D& tweenTo(const Point2D& other, s32 factor) {
        x = ::tweenTo(x, other.x, factor);
        y = ::tweenTo(y, other.y, factor);
        return *this;
    }

    bool distanceCheck(f32 range) {
        if (std::max(abs(f32ToS24q8(x)), abs(f32ToS24q8(y))) > f32ToS24q8(range))
            return false;
        return lengthSquared() < range * range;
    }

    f32 lengthSquared() {
        return x * x + y * y;
    }

    f32 length() {
        return sqrt(lengthSquared());
    }

    Point2D normalize() {
        Point2D point = *this;
        f32 len = length();
        if (len) {
            point.x /= len;
            point.y /= len;
        }
        return point;
    }

    Point2D& rotateXY(f32 rad){
        f32 cr  = cos(rad);
        f32 sr  = sin(rad);
        f32 x   = cr * this->x - sr * this->y;
        this->y = sr * this->x + cr * this->y;
        this->x = x;
        return *this;
    }

    Point2D operator - () {
        Point2D r;
        r.x = -x;
        r.y = -y;
        return r;
    }

    Point2D xx() {
        Point2D r;
        r.x = x;
        r.y = x;
        return r;
    }

    Point2D xy() {
        Point2D r;
        r.x = x;
        r.y = y;
        return r;
    }

    Point2D yx() {
        Point2D r;
        r.x = y;
        r.y = x;
        return r;
    }

    Point2D yy() {
        Point2D r;
        r.x = y;
        r.y = y;
        return r;
    }

    Point2D& operator *= (f32 scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Point2D& operator += (f32 scalar) {
        x += scalar;
        y += scalar;
        return *this;
    }

    Point2D& operator += (const Point2D &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Point2D& operator *= (const Point2D &other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Point2D& operator -= (const Point2D &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Point2D operator + (f32 scalar) const {
        Point2D c{*this};
        c.x += scalar;
        c.y += scalar;
        return c;
    }

    Point2D operator + (s32 scalar) const {
        Point2D c{*this};
        c.x += scalar;
        c.y += scalar;
        return c;
    }

    Point2D operator + (u32 scalar) const {
        Point2D c{*this};
        c.x += scalar;
        c.y += scalar;
        return c;
    }

    Point2D operator + (const Point2D &other) const {
        Point2D c{*this};
        c.x += other.x;
        c.y += other.y;
        return c;
    }

    Point2D operator * (const Point2D &other) const {
        Point2D c{*this};
        c.x *= other.x;
        c.y *= other.y;
        return c;
    }

    Point2D operator * (f32 scalar) const {
        Point2D c{*this};
        c.x *= scalar;
        c.y *= scalar;
        return c;
    }

    Point2D operator / (f32 scalar) const {
        Point2D c{*this};
        c.x /= scalar;
        c.y /= scalar;
        return c;
    }

    Point2D operator / (const Point2D &other) const {
        Point2D c{*this};
        c.x /= other.x;
        c.y /= other.y;
        return c;
    }

    Point2D operator - (f32 scalar) const {
        Point2D c{*this};
        c.x -= scalar;
        c.y -= scalar;
        return c;
    }

    Point2D operator - (s32 scalar) const {
        Point2D c{*this};
        c.x -= scalar;
        c.y -= scalar;
        return c;
    }

    Point2D operator - (u32 scalar) const {
        Point2D c{*this};
        c.x -= scalar;
        c.y -= scalar;
        return c;
    }

    Point2D operator - (const Point2D &other) const {
        Point2D c{*this};
        c.x -= other.x;
        c.y -= other.y;
        return c;
    }

    template <typename IterableCollection, typename Other>
    static void add(IterableCollection &points, const Other& other) {
        for (auto& point : points) {
            point += other;
        }
    }

    template <typename IterableCollection>
    static void rotateXY(IterableCollection &points, f32 rad) {
        f32 cr = cos(rad);
        f32 sr = sin(rad);
        for (auto& point : points) {
            f32 x = cr * point.x - sr * point.y;
            point.y = sr * point.x + cr * point.y;
            point.x = x;
        }
    }
};
