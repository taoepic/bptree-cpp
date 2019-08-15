#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <sys/time.h>
#include <unordered_set>
#include "bptree.hh"

#define MAXV 100000

void one_loop(std::unordered_set<long>& uset, 
			bptree<int, long>& bt) {
	long *v;
	struct timeval tv;

	gettimeofday(&tv, nullptr);
	srand(tv.tv_usec);
	std::cout << "---- inserting ----\n";
	for (long i = 0; i < MAXV; i++) {
		long k = (double)rand() / RAND_MAX * MAXV;

		if (!bt.find_key(k, v)) {
			if (uset.find(k) != uset.end()) {
				std::cout << "ins err: bt 0 uset 1 (" << k << ")\n";
				exit(-3);
			}
		//	std::cout << "insert " << k << std::endl;
			bt.insert_key(k, k * 2);
			uset.insert(k);
		} else {
			if (uset.find(k) == uset.end()) {
				std::cout << "ins err: bt 1 uset 0 (" << k << ")\n";
				exit(-1);
			}
		//	std::cout << "ins skip " << k << std::endl;
		}

		bt.check();
		if (i % 50000 == 1) {
			bt.dump_brief();
		//	std::cout << "uset size " << uset.size() << std::endl;
			if (bt.get_count() != uset.size()) {
				std::cout << "err: different size uset("  << 
					uset.size() << ") bt(" << bt.get_count() << ")\n";
				exit(-2);
			}
		}
	}

	std::cout << "---- deleting ----\n";
	for (long i = 0; i < MAXV; i++) {
		long k = (double)rand() / RAND_MAX * MAXV;

		if (bt.find_key(k, v)) {
			if (uset.find(k) == uset.end()) {
				std::cout << "del err: bt 1 uset 0 (" << k << ")\n";
				bt.dump();
				exit(-4);
			}
		//	std::cout << "delete " << k << std::endl;
			bt.delete_key(k);
			uset.erase(k);
		} else {
			if (uset.find(k) != uset.end()) {
				std::cout << "del err: bt 0 uset 1 (" << k << ")\n";
				bt.dump();
				exit(-5);
			}
		//	std::cout << "del skip " << k << std::endl;
		}

		bt.check();
		if (i % 50000 == 1) {
			bt.dump_brief();
		//	std::cout << "uset size " << uset.size() << std::endl;
			if (bt.get_count() != uset.size()) {
				std::cout << "err: different size uset(" <<
					uset.size() << ") bt(" << bt.get_count() << ")\n";
				exit(-2);
			}
		}
	}
}

int main() {
	std::unordered_set<long> uset;
	bptree<int, long> bt(128);
	for (int i = 0; i < 5000; i++) {
		std::cout << ">>> loop start " << i << std::endl;
		one_loop(uset, bt);
	}
	std::cout << "end." << std::endl;
}

