#include "ebr_set.h"


EbrLfSet::Accessor::Accessor(EbrLfSet* set):
    set { set },
    ebr_accessor { set->ebr.get_accessor() }
{

}

bool EbrLfSet::Accessor::add(int x) {
    return set->add(&ebr_accessor, x);
}

bool EbrLfSet::Accessor::remove(int x) {
    return set->remove(&ebr_accessor, x);
}

bool EbrLfSet::Accessor::contains(int x) {
    return set->contains(&ebr_accessor, x);
}


EbrLfSet::EbrLfSet(int max_threads):
    head { std::numeric_limits<int>::min() },
    tail { std::numeric_limits<int>::max() },
    ebr { max_threads }
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

void EbrLfSet::reset_accessor_counter() {
    ebr.reset_accessor_counter();
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

void EbrLfSet::find(Ebr::Accessor* ebr_accessor, int x, LfNode*& prev, LfNode*& curr) {
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
                    ebr_accessor->reuse(curr);
                    curr = succ;
                }
            } while(removed == true);

            while(curr->key >= x) return;
            prev = curr;
            curr = curr->next.get_ptr();
        }
    }
}

bool EbrLfSet::add(Ebr::Accessor* ebr_accessor, int x) {
    auto p = ebr_accessor->get_node(x);
    ebr_accessor->start_epoch();
    while(true) {
        LfNode* prev, * curr;
        find(ebr_accessor, x, prev, curr);

        if(curr->key == x) {
            ebr_accessor->end_epoch();
            delete p;
            return false;
        }
        else {
            p->next.set_ptr(curr);
            if(true == prev->next.cas(curr, p, false, false)) {
                ebr_accessor->end_epoch();
                return true;
            }
        }
    }
}

bool EbrLfSet::remove(Ebr::Accessor* ebr_accessor, int x) {
    ebr_accessor->start_epoch();
    while(true) {
        LfNode* prev, * curr;
        find(ebr_accessor, x, prev, curr);
        if(curr->key != x) {
            ebr_accessor->end_epoch();
            return false;
        }
        else {
            LfNode* succ = curr->next.get_ptr();
            if(false == curr->next.cas(succ, succ, false, true))
                continue;
            if(true == prev->next.cas(curr, succ, false, false))
                ebr_accessor->reuse(curr);
            ebr_accessor->end_epoch();
            return true;
        }
    }
}

bool EbrLfSet::contains(Ebr::Accessor* ebr_accessor, int x) {
    ebr_accessor->start_epoch();
    LfNode* curr = head.next.get_ptr();
    while(curr->key < x) {
        curr = curr->next.get_ptr();
    }
    bool result = (false == curr->next.get_removed()) && (curr->key == x);
    ebr_accessor->end_epoch();
    return result;
}
