#include "aabb.h"


AABB::AABB(const Vec3& center, const Vec3& extents):
    center { center },
    extents { extents }
{
}

AABB::AABB(const MinMaxAABB& minmax):
    center { (minmax.min + minmax.max) * 0.5f },
    extents { (minmax.max - minmax.min) * 0.5f }
{

}


MinMaxAABB::MinMaxAABB(const Vec3& min, const Vec3& max):
    min { min },
    max { max }
{

}

MinMaxAABB::MinMaxAABB(const AABB& aabb):
    min { aabb.center - aabb.extents },
    max { aabb.center + aabb.extents }
{

}


bool MinMaxAABB::intersects(const MinMaxAABB& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x &&
        min.y <= other.max.y && max.y >= other.min.y &&
        min.z <= other.max.z && max.z >= other.min.z);
}
