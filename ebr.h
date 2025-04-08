#ifndef __EBR_H__
#define __EBR_H__


#include <vector>
#include <queue>
#include <atomic>


class LfNode;

class Sptr {
private:
    std::atomic<long long> sptr;

public:
    Sptr();

public:
    void set_ptr(LfNode* ptr);
    LfNode* get_ptr();
    LfNode* get_ptr(bool* removed);
    bool get_removed();
    bool cas(LfNode* old_p, LfNode* new_p, bool old_m, bool new_m);
};

class LfNode {
public:
    int key;
    Sptr next;
    int ebr_number;

public:
    LfNode(const int& v);
};


struct alignas(64) AlignedAtomicInt {
    std::atomic_int value;
};

class alignas(64) FreeQueue : public std::queue<LfNode*> {
    
};

class Ebr {
private:
    std::atomic_int epoch_counter;
    std::vector<AlignedAtomicInt> epoch_array;
    std::vector<FreeQueue> free_queue;
    std::atomic_int free_index;

public:
    Ebr(int max_threads);
    ~Ebr();

public:
    void clear(int idx);
    void reuse(int idx, LfNode* node);
    void start_epoch(int idx);
    void end_epoch(int idx);

    LfNode* get_node(int idx, const int& x);

    class Accessor {
    private:
        Ebr& ebr;
        std::atomic_int idx;    // free queue에 접근하기 위한 인덱스

    public:
        Accessor(Ebr& ebr);
        Accessor(const Accessor& other);

    public:
        void clear();
        void reuse(LfNode* node);
        void start_epoch();
        void end_epoch();

        LfNode* get_node(const int& x);
    };
};


#endif