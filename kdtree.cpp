#include "kdtree.h"

#include <algorithm>


KDNode::KDNode(const Vec3& p, const MinMaxAABB& box):
    aabb_center { p },
    aabb { box }
{
    
}


KDTree::KDTree(const std::vector<AABB>& objects):
    root { build(objects, 0) }
{

}


void KDTree::query(const AABB& target, std::vector<AABB>& results) {
    search(root, target, results, 0);
}

KDNode* KDTree::build(std::vector<AABB> objects, int depth) {
    if(objects.empty()) return nullptr;

    // ���� ���̿� ���� ������ �� ���� (0=X, 1=Y, 2=Z)
    int axis = depth % 3;
    std::sort(objects.begin(), objects.end(), [axis](const auto& a, const auto& b) {
        return a.center[axis] < b.center[axis];
    });

    // �߰� ���� ����
    size_t mid = objects.size() / 2;
    KDNode* node = new KDNode { objects[mid].center, MinMaxAABB { objects[mid] } };

    // ���� �� ������ ����Ʈ�� ��� ����
    std::vector<AABB> left { objects.begin(), objects.begin() + mid };
    std::vector<AABB> right { objects.begin() + mid + 1, objects.end() };

    node->left = build(left, depth + 1);
    node->right = build(right, depth + 1);

    return node;
}

void KDTree::search(KDNode* node, const AABB& target, std::vector<AABB>& results, int depth) {
    if(!node) return;

    // ���� ���� AABB �浹 �˻�
    if(node->aabb.intersects(target)) {
        results.push_back(node->aabb);
    }

    // �˻��� �� ����
    int axis = depth % 3;
    float targetValue = target.center[axis];
    float nodeValue = node->aabb_center[axis];

    // ���� �� ������ ����Ʈ�� �˻�
    if(targetValue <= nodeValue) search(node->left, target, results, depth + 1);
    if(targetValue >= nodeValue) search(node->right, target, results, depth + 1);
}
