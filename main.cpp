#include <iostream>
#include <functional>
#include <map>
#include <cstring>
#include <fstream>

#include "jit.h"

JIT_DECLARATION
(
		difficult,
		template<typename T>
		void difficult() {

		}
)

int main(int argc, char **argv) {
	if (argc > 2) {
		JIT_CALL(difficult, difficult<bool>);
	} else {
		JIT_CALL(difficult, difficult<int>);
	}
	return 0;
}