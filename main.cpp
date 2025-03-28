#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "kdtree.h"


int main() {
    // KD-Tree에 저장할 AABB 데이터 (중심점, AABB)
    std::vector<AABB> objects = {
        {{2, 3, 5}, {1, 1, 1}},
        {{5, 5, 5}, {1, 1, 1}},
        {{8, 8, 8}, {5, 5, 5}}  // 이거 찾아야되는데 못찾는다.
    };

    // KD-Tree 생성
    KDTree tree(objects);

    // 충돌 검사할 대상 AABB
    AABB queryBox { { 4, 4, 4 }, {1, 1, 1} };
    std::vector<AABB> results;

    // KD-Tree에서 충돌 후보 검색
    tree.query(queryBox, results);

    // 결과 출력
    std::cout << "충돌한 AABB 개수: " << results.size() << std::endl;
    for(const auto& box : results) {
        std::cout << "충돌한 AABB: " << box.center.x << ", " << box.center.y << ", " << box.center.z << std::endl;
    }

    return 0;
}