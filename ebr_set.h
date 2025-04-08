#pragma once

#include "ebr.h"


class EbrLfSet {
private:
    LfNode head;
    LfNode tail;
    Ebr ebr;
    std::atomic_int accessor_count;

public:
    class Accessor {
    private:
        EbrLfSet* set;
        int accessor_idx;

    private:
        Accessor(EbrLfSet* set);

    public:
        bool add(int x);
        bool remove(int x);
        bool contains(int x);

        friend class EbrLfSet;
    };

public:
    EbrLfSet(int max_threads);

public:
    void clear();
    void reset_accessor_count();
    /// thread-safe하지 않은 contains
    bool contains(int x);
    Accessor get_accessor();

private:
    void find(int idx, int x, LfNode*& prev, LfNode*& curr);
    bool add(int idx, int x);
    bool remove(int idx, int x);
    /// thread-safe한 contains
    bool contains(int idx, int x);
};
