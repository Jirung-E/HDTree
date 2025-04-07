#include "hdtree.h"

#include <limits>
#include <atomic>


// Fig. 6
constexpr double INF1 = std::numeric_limits<double>::max() - 1;
constexpr double INF2 = std::numeric_limits<double>::max() - 2;
constexpr double INF3 = std::numeric_limits<double>::max();
constexpr double INF4 = std::numeric_limits<double>::max() - 4;
constexpr double INF5 = std::numeric_limits<double>::max() - 3;

enum RemoveMode {
    INJECTION,
    CLEANUP,
};


Node::Node(int d, double c, double fill): 
    Node { d, c, fill, nullptr, nullptr, nullptr }
{

}

Node::Node(int d, double c, double fill, Node* lt, Node* rt, Node* pr):
    d { d }, c { c }, k { }, lt { lt }, rt { rt }, pr { pr } 
{
    k.fill(fill);
}

bool Node::cas_child(Node* old_node, ptrdiff_t old_lsb, Node* new_node, ptrdiff_t new_lsb, bool ch_dir) {
    ptrdiff_t old_ptr = reinterpret_cast<ptrdiff_t>(old_node) | old_lsb;
    ptrdiff_t new_ptr = reinterpret_cast<ptrdiff_t>(new_node) | new_lsb;
    return (ch_dir) ? cas(&lt, old_ptr, new_ptr) : cas(&rt, old_ptr, new_ptr);
}

bool Node::cas(Node** node, ptrdiff_t old_ptr, ptrdiff_t new_ptr) {
    std::atomic<ptrdiff_t>* node_p = reinterpret_cast<std::atomic<ptrdiff_t>*>(node);
    return std::atomic_compare_exchange_strong(node_p, &old_ptr, new_ptr);
}


HDTree::HDTree():
    root { new Node { 0, INF1, 0.0 } }, 
    nncs { }
{
    root->rt = new Node { 0, 0.0, INF3 };
    root->lt = new Node { 0, INF2, 0.0 };
    root->lt->lt = new Node { 0, 0.0, INF4 };
    root->lt->rt = new Node { 0, 0.0, INF5 };
}

bool HDTree::contains(double k[DIMENSION]) {
    start_op();
    search_field f;
    search(f, k);
    if(!isMark(f.cr)) {
        if(isEqual(k, f.cr->k)) {
            sync(f.pr, f.cr);
            end_op();
            return true;
        }
    }
    end_op();
    return false;
}

bool HDTree::insert(double k[DIMENSION]) {
    start_op();
    search_field f;
    while(true) {
        search(f, k);
        if(!isEqual(k, f.cr->k)) {
            Node* new_internal = new_node();
            Node* new_leaf = new_node();
            new_leaf->setKey(k);
            new_leaf->pr = new_internal;
            int dim = getMinDiffDimension(new_leaf->k, f.cr->k);
            new_internal->d = dim;
            new_internal->c = (new_leaf->k[dim] + f.cr->k[dim]) / 2;
            new_internal->pr = f.pr;
            bool new_leaf_dir = new_leaf->k[dim] < f.cr->k[dim];
            if(new_leaf_dir) {
                new_internal->lt = new_leaf;
                new_internal->rt = f.cr;
            }
            else {
                new_internal->lt = f.cr;
                new_internal->rt = new_leaf;
            }
            if(f.pr->cas_child(f.cr, nullptr, new_internal, nullptr, f.cr_dir)) {
                cas(&f.cr->pr, f.pr, new_internal);
                f.pr = new_internal;
                f.cr = new_leaf;
                f.cr_dir = new_leaf_dir;
                sync(f.pr, f.cr);
                end_op();
                return true;
            }
            else {
                del_node(new_leaf);
                del_node(new_internal);
                Node* child = f.pr->getChild(f.cr_dir);
                if((child == f.cr) && (isMark(child) || isTag(child))) {
                    cleanup(f);
                }
            }
        }
        else {
            end_op();
            return false;
        }
    }
}

bool HDTree::remove(double k[DIMENSION]) {
    start_op();
    search_field f;
    RemoveMode mode = INJECTION;
    Node* del = nullptr;
    while(true) {
        search(f, k);
        if(INJECTION == mode) {
            if(!isEqual(k, f.cr->k)) {
                end_op();
                return false;
            }
            del = f.cr;
            if(f.pr->cas_child(f.cr, nullptr, f.cr, MARK, f.cr_dir)) {
                mod = CLEANUP;
                if(cleanup(f)) {
                    end_op();
                    return true;
                }
            }
            else {
                Node* child = f.pr->getChild(f.cr_dir);
                if((child == f.cr) && (isMark(child) || isTag(child))) {
                    cleanup(f);
                }
            }
        }
        else {
            if(del != f.cr) {
                end_op();
                return true;
            }
            else if(cleanup(f)) {
                end_op();
                return true;
            }
        }
    }
}

void HDTree::search(search_field& f, double k[DIMENSION]) {
    f.ac = root;
    f.sc = root->lt;
    f.sc_dir = true;
    f.pr = f.sc;
    f.cr = f.pr->lt;
    f.cr_dir = true;
    while(!f.cr->isLeaf()) {
        if(!isTag(f.pr)) {
            f.ac = f.pr;
            f.sc = f.cr;
            f.sc_dir = f.cr_dir;
        }
        f.pr = f.cr;
        f.cr_dir = k[f.pr->d] < f.pr->c;
        f.cr = f.pr->getChild(f.cr_dir);
    }
}

bool HDTree::cleanup(search_field& f) {
    Node* child = f.pr->getChild(f.cr_dir);
    Node* sibling = f.pr->getChild(!f.cr_dir);
    if(!isMark(child)) {
        sibling = child;
        f.cr_dir = !f.cr_dir;
    }
    if(!isTag(sibling)) {
        f.pr->cas_child(sibling, lsb(sibling), sibling, lsb(sibling) | TAG, !f.cr_dir);
    }
    sibling = f.pr->getChild(!f.cr_dir);
    if(!isTag(sibling)) {
        return false;
    }
    if(f.ac->cas_child(f.sc, nullptr, lsb(sibling), lsb(sibling) & MARK, f.sc_dir)) {
        cas(&sibling->pr, f.pr, f.ac);
        retire_subtree(f.sc, sibling);
        return true;
    }
    return false;
}


