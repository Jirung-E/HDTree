#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "kdtree.h"


int main() {
    // KD-Tree�� ������ AABB ������ (�߽���, AABB)
    std::vector<AABB> objects = {
        {{2, 3, 5}, {1, 1, 1}},
        {{5, 5, 5}, {1, 1, 1}},
        {{8, 8, 8}, {5, 5, 5}}  // �̰� ã�ƾߵǴµ� ��ã�´�.
    };

    // KD-Tree ����
    KDTree tree(objects);

    // �浹 �˻��� ��� AABB
    AABB queryBox { { 4, 4, 4 }, {1, 1, 1} };
    std::vector<AABB> results;

    // KD-Tree���� �浹 �ĺ� �˻�
    tree.query(queryBox, results);

    // ��� ���
    std::cout << "�浹�� AABB ����: " << results.size() << std::endl;
    for(const auto& box : results) {
        std::cout << "�浹�� AABB: " << box.center.x << ", " << box.center.y << ", " << box.center.z << std::endl;
    }

    return 0;
}