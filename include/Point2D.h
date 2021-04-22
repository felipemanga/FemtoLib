#pragma once

#include "types.hpp"

struct Point2D {
    f32 x, y;
    void set(f32 x, f32 y){
        this->x = x;
        this->y = y;
    }

    void rotateXY(f32 rad){
        f32 cr  = cos(rad);
        f32 sr  = sin(rad);
        f32 x   = cr * this->x - sr * this->y;
        this->y = sr * this->x + cr * this->y;
        this->x = x;
    }

    Point2D& operator *= (f32 scalar) {
        x *= scalar;
        y *= scalar;
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


    Point2D operator + (const Point2D &other) {
        Point2D c{*this};
        c.x += other.x;
        c.y += other.y;
        return c;
    }

    Point2D operator * (const Point2D &other) {
        Point2D c{*this};
        c.x *= other.x;
        c.y *= other.y;
        return c;
    }

    Point2D operator - (const Point2D &other) {
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
