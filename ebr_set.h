#pragma once

#include "ebr.h"


class EbrLfSet {
private:
    LfNode head;
    LfNode tail;
    Ebr& ebr;

public:
    EbrLfSet(Ebr& ebr);

public:
    void clear();
    void find(Ebr::Accessor& ebr, int x, LfNode*& prev, LfNode*& curr);
    bool add(Ebr::Accessor& ebr, int x);
    bool remove(Ebr::Accessor& ebr, int x);
    bool contains(Ebr::Accessor& ebr, int x);

    class Accessor {
    private:
        EbrLfSet& set;
        Ebr::Accessor ebr_accessor;

    public:
        Accessor(EbrLfSet& set);
        Accessor(const Accessor& other);

    public:
        void find(int x, LfNode*& prev, LfNode*& curr);
        bool add(int x);
        bool remove(int x);
        bool contains(int x);
    };

    Accessor get_accessor();
};
