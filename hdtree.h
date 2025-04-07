#ifndef __HDTREE_H__
#define __HDTREE_H__


#include "ebr.h"

#include <array>


constexpr int DIMENSION = 2;
constexpr int MAX_THREADS = 16;


class Node {
public:
    int d;
    double c;
    std::array<double, DIMENSION> k;
    Node* lt;
    Node* rt;
    Node* pr;

public:
    Node(int d, double c, double fill);
    Node(int d, double c, double fill, Node* lt, Node* rt, Node* pr);

public:
    bool cas_child(Node* old_node, ptrdiff_t old_lsb, Node* new_node, ptrdiff_t new_lsb, bool ch_dir);
    static bool cas(Node** node, ptrdiff_t old_ptr, ptrdiff_t new_ptr);
};

class Neighbor {
public:
    Node* p;
    double d;
};

class NNCollector {
public:
    double tgt[DIMENSION];
    Neighbor* cn;
    Neighbor* rn;
};

struct search_field {
    Node* ac;
    Node* sc;
    Node* pr;
    Node* cr;

    bool sc_dir;
    bool cr_dir;
};

struct seek_field {
    Node* pr;
    Node* cr;
    bool cr_dir;
    double hi[DIMENSION];
    double lo[DIMENSION];
};


class HDTree {
private:
    Node* root;
    std::array<NNCollector*, MAX_THREADS> nncs;
    Ebr ebr;

public:
    HDTree();

public:
    bool contains(double k[DIMENSION]);
    bool insert(double k[DIMENSION]);
    bool remove(double k[DIMENSION]);
    double nns(double k[DIMENSION]);

private:
    void search(search_field& f, double k[DIMENSION]);
    bool cleanup(search_field& f);
};


#endif