#include "ebr_set.h"

#include <iostream>
#include <array>
#include <thread>
#include <vector>
#include <chrono>


const int NUM_TEST = 4'000'000;
const int KEY_RANGE = 1000;
const int MAX_THREADS = 12;	// 현재컴퓨터가 6코어 12스레드임
const int NUM_THREADS_SET[] { 1, 2, 3, 4, 6, 12 };


class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re): op(o), i_value(i), o_value(re) { }
};

std::array<std::vector<HISTORY>, MAX_THREADS> history;

void worker_check(EbrLfSet::Accessor* my_set, int num_threads, int th_id) {
	for(int i = 0; i < NUM_TEST / num_threads; ++i) {
		int op = rand() % 3;
		switch(op) {
			case 0: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(0, v, my_set->add(v));
				break;
			}
			case 1: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(1, v, my_set->remove(v));
				break;
			}
			case 2: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(2, v, my_set->contains(v));
				break;
			}
		}
	}
}

void check_history(EbrLfSet* my_set, int num_threads) {
	std::array <int, KEY_RANGE> survive = {};
	std::cout << "Checking Consistency : ";
	if(history[0].size() == 0) {
		std::cout << "No history.\n";
		return;
	}
	for(int i = 0; i < num_threads; ++i) {
		for(auto& op : history[i]) {
			if(false == op.o_value) continue;
			if(op.op == 3) continue;
			if(op.op == 0) survive[op.i_value]++;
			if(op.op == 1) survive[op.i_value]--;
		}
	}
	for(int i = 0; i < KEY_RANGE; ++i) {
		int val = survive[i];
		if(val < 0) {
			std::cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if(val > 1) {
			std::cout << "ERROR. The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if(val == 0) {
			if(my_set->contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if(val == 1) {
			if(false == my_set->contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}


void benchmark(EbrLfSet::Accessor* my_set, const int th_id, const int num_thread) {
	int key;

	for(int i = 0; i < NUM_TEST / num_thread; i++) {
		switch(rand() % 3) {
			case 0: key = rand() % KEY_RANGE;
				my_set->add(key);
				break;
			case 1: key = rand() % KEY_RANGE;
				my_set->remove(key);
				break;
			case 2: key = rand() % KEY_RANGE;
				my_set->contains(key);
				break;
			default: std::cout << "Error\n";
				exit(-1);
		}
	}
}


void test_history() {
    using namespace std::chrono;

	EbrLfSet my_set { MAX_THREADS };

	//for(int n = 1; n <= MAX_THREADS; n = n * 2) {
	for(int n : NUM_THREADS_SET) {
		my_set.clear();
		my_set.reset_accessor_counter();
		for(auto& v : history)
			v.clear();
		std::vector<std::thread> tv;
		std::vector<EbrLfSet::Accessor*> accessors;
		for(int i = 0; i < n; ++i) {
			accessors.emplace_back(my_set.new_accessor());
		}
		auto start_t = high_resolution_clock::now();
		for(int i = 0; i < n; ++i) {
			tv.emplace_back(worker_check, accessors[i], n, i);
		}
		for(auto& th : tv) {
			th.join();
		}
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms.";
		check_history(&my_set, n);
        for(auto& accessor : accessors) {
            delete accessor;
        }
	}

	for(auto& v : history) {
		v.clear();
		v.shrink_to_fit();
	}
}


void test() {
    using namespace std::chrono;

	//for(int n = 1; n <= MAX_THREADS; n = n * 2) {
    for(int n : NUM_THREADS_SET) {
		EbrLfSet my_set { n };
		std::vector<std::thread> tv;
		std::vector<EbrLfSet::Accessor*> accessors;
		for(int i = 0; i < n; ++i) {
			accessors.emplace_back(my_set.new_accessor());
		}
		auto start_t = high_resolution_clock::now();
		for(int i = 0; i < n; ++i) {
			tv.emplace_back(benchmark, accessors[i], i, n);
		}
		for(auto& th : tv) {
			th.join();
		}
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
        printf("%d Threads,  %zu ms.\n", n, ms);
        for(auto& accessor : accessors) {
            delete accessor;
        }
	}
}


int main() {
	using namespace std::chrono;

	test_history();

	while(true) {
		const int parallel_set_count = 1;
		std::vector<std::thread> test_threads;
		test_threads.reserve(parallel_set_count);
		for(int i=0; i<parallel_set_count; ++i) {
			test_threads.emplace_back(test);
		}

		for(auto& th : test_threads) {
			th.join();
		}
	}
}