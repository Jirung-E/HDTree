#include "ebr_set.h"


EbrLfSet::Accessor::Accessor(EbrLfSet* set):
    set { set },
    accessor_idx { set->accessor_count++ }
{

}

bool EbrLfSet::Accessor::add(int x) {
    return set->add(accessor_idx, x);
}

bool EbrLfSet::Accessor::remove(int x) {
    return set->remove(accessor_idx, x);
}

bool EbrLfSet::Accessor::contains(int x) {
    return set->contains(accessor_idx, x);
}


EbrLfSet::EbrLfSet(int max_threads):
    head { std::numeric_limits<int>::min() },
    tail { std::numeric_limits<int>::max() },
    ebr { max_threads },
    accessor_count { 0 }
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

void EbrLfSet::reset_accessor_count() {
    accessor_count = 0;
}

bool EbrLfSet::contains(int x) {
    LfNode* curr = head.next.get_ptr();
    while(curr->key < x) {
        curr = curr->next.get_ptr();
    }
    bool result = (false == curr->next.get_removed()) && (curr->key == x);
    return result;
}

EbrLfSet::Accessor EbrLfSet::get_accessor() {
    return Accessor { this };
}

void EbrLfSet::find(int idx, int x, LfNode*& prev, LfNode*& curr) {
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
                    ebr.reuse(idx, curr);
                    curr = succ;
                }
            } while(removed == true);

            while(curr->key >= x) return;
            prev = curr;
            curr = curr->next.get_ptr();
        }
    }
}

bool EbrLfSet::add(int idx, int x) {
    auto p = ebr.get_node(idx, x);
    ebr.start_epoch(idx);
    while(true) {
        LfNode* prev, * curr;
        find(idx, x, prev, curr);

        if(curr->key == x) {
            ebr.end_epoch(idx);
            delete p;
            return false;
        }
        else {
            p->next.set_ptr(curr);
            if(true == prev->next.cas(curr, p, false, false)) {
                ebr.end_epoch(idx);
                return true;
            }
        }
    }
}

bool EbrLfSet::remove(int idx, int x) {
    ebr.start_epoch(idx);
    while(true) {
        LfNode* prev, * curr;
        find(idx, x, prev, curr);
        if(curr->key != x) {
            ebr.end_epoch(idx);
            return false;
        }
        else {
            LfNode* succ = curr->next.get_ptr();
            if(false == curr->next.cas(succ, succ, false, true))
                continue;
            if(true == prev->next.cas(curr, succ, false, false))
                ebr.reuse(idx, curr);
            ebr.end_epoch(idx);
            return true;
        }
    }
}

bool EbrLfSet::contains(int idx, int x) {
    ebr.start_epoch(idx);
    LfNode* curr = head.next.get_ptr();
    while(curr->key < x) {
        curr = curr->next.get_ptr();
    }
    bool result = (false == curr->next.get_removed()) && (curr->key == x);
    ebr.end_epoch(idx);
    return result;
}
