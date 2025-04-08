#include "ebr.h"

#include <iostream>
#include <queue>


thread_local int thread_id;


Sptr::Sptr(): sptr { 0 } {

}


void Sptr::set_ptr(LfNode* ptr) {
    sptr = reinterpret_cast<long long>(ptr);
}

LfNode* Sptr::get_ptr() {
    return reinterpret_cast<LfNode*>(sptr & 0xFFFFFFFFFFFFFFFE);
}

LfNode* Sptr::get_ptr(bool* removed) {
    long long p = sptr;
    *removed = (1 == (p & 1));
    return reinterpret_cast<LfNode*>(p & 0xFFFFFFFFFFFFFFFE);
}

bool Sptr::get_removed() {
    return (1 == (sptr & 1));
}

bool Sptr::cas(LfNode* old_p, LfNode* new_p, bool old_m, bool new_m) {
    long long old_v = reinterpret_cast<long long>(old_p);
    if(true == old_m) old_v = old_v | 1;
    else
        old_v = old_v & 0xFFFFFFFFFFFFFFFE;
    long long new_v = reinterpret_cast<long long>(new_p);
    if(true == new_m) new_v = new_v | 1;
    else
        new_v = new_v & 0xFFFFFFFFFFFFFFFE;
    return std::atomic_compare_exchange_strong(&sptr, &old_v, new_v);
}


LfNode::LfNode(const int& v):
    key { v },
    ebr_number { 0 }
{

}


thread_local std::queue<LfNode*> m_free_queue;


Ebr::Ebr(int max_threads):
    epoch_counter { 1 },
    epoch_array(max_threads)
{

}

Ebr::~Ebr() {
    clear();
}

void Ebr::clear() {
    while(false == m_free_queue.empty()) {
        delete m_free_queue.front();
        m_free_queue.pop();
    }
    epoch_counter = 1;
}

void Ebr::reuse(LfNode* node) {
    node->ebr_number = epoch_counter;
    m_free_queue.push(node);
}
void Ebr::start_epoch() {
    int epoch = epoch_counter++;
    epoch_array[thread_id].value = epoch;
}
void Ebr::end_epoch() {
    epoch_array[thread_id].value = 0;
}

LfNode* Ebr::get_node(const int& x) {
    if(true == m_free_queue.empty()) {
        return new LfNode { x };
    }

    LfNode* p = m_free_queue.front();
    for(auto& ea : epoch_array) {
        int epoch = ea.value;
        if((epoch != 0) && (epoch < p->ebr_number)) {
            return new LfNode { x };
        }
    }
    m_free_queue.pop();
    p->key = x;
    std::atomic_thread_fence(std::memory_order_seq_cst);
    p->next.set_ptr(nullptr);
    return p;
}
