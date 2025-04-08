#pragma once

#include "ebr.h"


class EbrLfSet {
private:
    LfNode head;
    LfNode tail;
    Ebr ebr;

public:
    EbrLfSet(int max_threads);

public:
    void clear();
    void find(int x, LfNode*& prev, LfNode*& curr);
    bool add(int x);
    bool remove(int x);
    bool contains(int x);
};
