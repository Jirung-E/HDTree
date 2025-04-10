#pragma once

#include "ebr.h"


class EbrLfSet {
private:
    LfNode head;
    LfNode tail;
    Ebr ebr;

public:
    class Accessor {
    private:
        EbrLfSet* set;
        Ebr::Accessor ebr_accessor;

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
    ~EbrLfSet();

public:
    void clear();
    void reset_accessor_counter();
    /// thread-safe���� ���� contains
    bool contains(int x);
    Accessor* new_accessor();

private:
    void find(Ebr::Accessor* ebr_accessor, int x, LfNode*& prev, LfNode*& curr);
    bool add(Ebr::Accessor* ebr_accessor, int x);
    bool remove(Ebr::Accessor* ebr_accessor, int x);
    /// thread-safe�� contains
    bool contains(Ebr::Accessor* ebr_accessor, int x);
};
