#ifndef __VEC3_H__
#define __VEC3_H__


class Vec3 {
public:
    union {
        struct {
            float x;
            float y;
            float z;
        };
        float _data[3];
    };

public:
    float& operator[](int index) {
        return _data[index];
    }

    const float& operator[](int index) const {
        return _data[index];
    }

    Vec3 operator+(const Vec3& rhs) const {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    Vec3 operator-(const Vec3& rhs) const {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    Vec3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }
};


#endif