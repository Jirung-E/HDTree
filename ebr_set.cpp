#include "ebr_set.h"


EbrLfSet::EbrLfSet():
    head { std::numeric_limits<int>::min() },
    tail { std::numeric_limits<int>::max() },
    ebr { 16 }
{
    head.next.set_ptr(&tail);
}


void EbrLfSet::clear() {
    while(head.next.get_ptr() != &tail) {
        auto p = head.next.get_ptr();
        head.next.set_ptr(p->next.get_ptr());
        delete p;
    }

    ebr.clear();
}

void EbrLfSet::find(int x, LfNode*& prev, LfNode*& curr) {
    while(true) {
    retry:
        prev = &head;
        curr = prev->next.get_ptr();

        while(true) {
            bool removed = false;
            do {
                LfNode* succ = curr->next.get_ptr(&removed);
                if(removed == true) {
                    if(false == prev->next.cas(curr, succ, false, false)) goto retry;
                    ebr.reuse(curr);
                    curr = succ;
                }
            } while(removed == true);

            while(curr->key >= x) return;
            prev = curr;
            curr = curr->next.get_ptr();
        }
    }
}

bool EbrLfSet::add(int x) {
    auto p = ebr.get_node(x);
    ebr.start_epoch();
    while(true) {
        LfNode* prev, * curr;
        find(x, prev, curr);

        if(curr->key == x) {
            ebr.end_epoch();
            delete p;
            return false;
        }
        else {
            p->next.set_ptr(curr);
            if(true == prev->next.cas(curr, p, false, false)) {
                ebr.end_epoch();
                return true;
            }
        }
    }
}

bool EbrLfSet::remove(int x) {
    ebr.start_epoch();
    while(true) {
        LfNode* prev, * curr;
        find(x, prev, curr);
        if(curr->key != x) {
            ebr.end_epoch();
            return false;
        }
        else {
            LfNode* succ = curr->next.get_ptr();
            if(false == curr->next.cas(succ, succ, false, true))
                continue;
            if(true == prev->next.cas(curr, succ, false, false))
                ebr.reuse(curr);
            ebr.end_epoch();
            return true;
        }
    }
}

bool EbrLfSet::contains(int x) {
    ebr.start_epoch();
    LfNode* curr = head.next.get_ptr();
    while(curr->key < x) {
        curr = curr->next.get_ptr();
    }
    bool result = (false == curr->next.get_removed()) && (curr->key == x);
    ebr.end_epoch();
    return result;
}
