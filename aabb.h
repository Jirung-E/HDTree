#ifndef __AABB_H__
#define __AABB_H__


#include "vec3.h"


class AABB;
class MinMaxAABB;


class AABB {
public:
    Vec3 center;
    Vec3 extents;

public:
    AABB(const Vec3& center, const Vec3& extents);
    AABB(const MinMaxAABB& minmax);
};


class MinMaxAABB {
public:
    Vec3 min;
    Vec3 max;

public: 
    MinMaxAABB(const Vec3& min, const Vec3& max);
    MinMaxAABB(const AABB& aabb);

public:
    bool intersects(const MinMaxAABB& other) const;
};


#endif