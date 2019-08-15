#include <iostream>
#include <string>
#include "bptree.hh"

void loop(bptree<long,std::string>& bt) {
	long values[] = {
		11, 13, 17, 80, 33, 50,
		6, 10, 55, 34, 90, 92, 94,
		98, 29, 27, 26, 24, 25, 20,
		66, 21, 88, 28, 77, 54, 31,
		9, 99, 1, 71, 2, 97, 5, 3,
	};

	long values2[] = {
		98, 29, 27, 26, 24, 25, 20,
		9, 99, 1, 71, 2, 97, 5, 3,
		6, 10, 55, 34, 90, 92, 94,
		66, 21, 88, 28, 77, 54, 31,
		11, 13, 17, 80, 33, 50,
	};

	for (int i = 0; i < sizeof(values) / sizeof(long); i++) {
		std::cout << "---- insert " << values[i] << "-----\n";
		char buf[40];
		sprintf(buf, "[[%ld]]", values[i]);
		std::string s{buf};
		bt.insert_key(values[i], s);
		bt.dump();
		bt.dump_leaf_keys();
	}
	
	bt.dump();
	bt.dump_leaf_keys();

	for (int i = 0; i < sizeof(values2) / sizeof(long); i++) {
		std::cout << "---- delete " << values2[i] << "-----\n";
		bt.delete_key(values2[i]);
		bt.dump();
		bt.dump_leaf_keys();
	}
	
	bt.dump();
	bt.dump_leaf_keys();
}

int main() {
	bptree<long, std::string> bt(4);
	for (int i = 0; i < 1; i++)
		loop(bt);
}

