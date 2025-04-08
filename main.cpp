#include "ebr.h"
#include "ebr_set.h"

#include <iostream>
#include <array>
#include <thread>
#include <vector>
#include <chrono>


const int NUM_TEST = 400'000;
const int KEY_RANGE = 1000;
const int MAX_THREADS = 16;


class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re): op(o), i_value(i), o_value(re) { }
};

std::array<std::vector<HISTORY>, 16> history;

void worker_check(EbrLfSet::Accessor* accessor, int num_threads, int th_id) {
	for(int i = 0; i < NUM_TEST / num_threads; ++i) {
		int op = rand() % 3;
		switch(op) {
			case 0: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(0, v, accessor->add(v));
				break;
			}
			case 1: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(1, v, accessor->remove(v));
				break;
			}
			case 2: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(2, v, accessor->contains(v));
				break;
			}
		}
	}
}

void check_history(EbrLfSet::Accessor* accessor, int num_threads) {
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
			if(accessor->contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if(val == 1) {
			if(false == accessor->contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}


void benchmark(EbrLfSet::Accessor* accessor, const int th_id, const int num_thread) {
	int key;

	for(int i = 0; i < NUM_TEST / num_thread; i++) {
		switch(rand() % 3) {
			case 0: key = rand() % KEY_RANGE;
				accessor->add(key);
				break;
			case 1: key = rand() % KEY_RANGE;
				accessor->remove(key);
				break;
			case 2: key = rand() % KEY_RANGE;
				accessor->contains(key);
				break;
			default: std::cout << "Error\n";
				exit(-1);
		}
	}
}


int main() {
	using namespace std::chrono;

	Ebr ebr { MAX_THREADS };
	EbrLfSet my_set { ebr };

	//for(int n = 1; n <= MAX_THREADS; n = n * 2) {
	//	my_set.clear();
	//	for(auto& v : history)
	//		v.clear();
	//	std::vector<std::thread> tv;
	//	auto start_t = high_resolution_clock::now();
	//	EbrLfSet::Accessor* accessor_for_history = nullptr;
	//	for(int i = 0; i < n; ++i) {
 //           EbrLfSet::Accessor accessor { my_set };
 //           if(i == 0) {
 //               accessor_for_history = &accessor;
 //           }
	//		tv.emplace_back(worker_check, &accessor, n, i);
	//	}
	//	for(auto& th : tv)
	//		th.join();
	//	auto end_t = high_resolution_clock::now();
	//	auto exec_t = end_t - start_t;
	//	size_t ms = duration_cast<milliseconds>(exec_t).count();
	//	std::cout << n << " Threads,  " << ms << "ms.";
	//	check_history(accessor_for_history, n);
	//}

	//for(auto& v : history) {
	//	v.clear();
	//	v.shrink_to_fit();
	//}

	for(int n = 1; n <= MAX_THREADS; n = n * 2) {
		my_set.clear();
		std::vector<std::thread> tv;
		auto start_t = high_resolution_clock::now();
		for(int i = 0; i < n; ++i) {
			auto accessor = my_set.get_accessor();
			tv.emplace_back(benchmark, &accessor, i, n);
		}
		for(auto& th : tv)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms." << std::endl;
	}
}