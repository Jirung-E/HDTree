#include "ebr_set.h"


EbrLfSet::EbrLfSet(Ebr& ebr):
    head { std::numeric_limits<int>::min() },
    tail { std::numeric_limits<int>::max() },
    ebr { ebr }
{
    head.next.set_ptr(&tail);
}


void EbrLfSet::clear() {
    while(head.next.get_ptr() != &tail) {
        auto p = head.next.get_ptr();
        head.next.set_ptr(p->next.get_ptr());
        delete p;
    }

    ebr.clear(0);
}

void EbrLfSet::find(Ebr::Accessor& ebr, int x, LfNode*& prev, LfNode*& curr) {
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

bool EbrLfSet::add(Ebr::Accessor& ebr, int x) {
    auto p = ebr.get_node(x);
    ebr.start_epoch();
    while(true) {
        LfNode* prev, * curr;
        find(ebr, x, prev, curr);

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

bool EbrLfSet::remove(Ebr::Accessor& ebr, int x) {
    ebr.start_epoch();
    while(true) {
        LfNode* prev, * curr;
        find(ebr, x, prev, curr);
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

bool EbrLfSet::contains(Ebr::Accessor& ebr, int x) {
    ebr.start_epoch();
    LfNode* curr = head.next.get_ptr();
    while(curr->key < x) {
        curr = curr->next.get_ptr();
    }
    bool result = (false == curr->next.get_removed()) && (curr->key == x);
    ebr.end_epoch();
    return result;
}


EbrLfSet::Accessor::Accessor(EbrLfSet& set):
    set { set },
    ebr_accessor { set.ebr }
{

}

void EbrLfSet::Accessor::clear() {
    set.clear();
}

void EbrLfSet::Accessor::find(int x, LfNode*& prev, LfNode*& curr) {
    set.find(ebr_accessor, x, prev, curr);
}

bool EbrLfSet::Accessor::add(int x) {
    return set.add(ebr_accessor, x);
}

bool EbrLfSet::Accessor::remove(int x) {
    return set.remove(ebr_accessor, x);
}

bool EbrLfSet::Accessor::contains(int x) {
    return set.contains(ebr_accessor, x);
}