#ifndef __KDTREE_H__
#define __KDTREE_H__


#include "vec3.h"
#include "aabb.h"

#include <vector>


class KDNode {
public:
    Vec3 aabb_center;
    MinMaxAABB aabb;
    KDNode* left = nullptr;
    KDNode* right = nullptr;

public:
    KDNode(const Vec3& p, const MinMaxAABB& box);
};


class KDTree {
private:
    KDNode* root = nullptr;

public:
    KDTree(const std::vector<AABB>& objects);

public:
    // 범위 내 AABB 충돌 검사
    void query(const AABB& target, std::vector<AABB>& results);

private:
    // KD-Tree 빌드 (재귀적 구성)
    static KDNode* build(std::vector<AABB> objects, int depth);

    // KD-Tree 검색 (재귀적 범위 쿼리)
    static void search(KDNode* node, const AABB& target, std::vector<AABB>& results, int depth);
};


#endif