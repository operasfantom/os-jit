#ifndef OS_JIT_JIT_H
#define OS_JIT_JIT_H

#include <functional>
#include <string_view>
#include <dlfcn.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cassert>
#include <cstring>
#include <dlfcn.h>
#include <cstddef>
#include <algorithm>
#include <csignal>
#include <csetjmp>

namespace rd_jit {
	using std::string_literals::operator ""s;

	int compile(std::string_view name_view, std::string_view call_view, std::string_view code_view);

	template<typename F>
	class functor {
	private:
		int lib_fd;
		size_t lib_size;

		void *fun;

		void free_map() {
			munmap(fun, lib_size);
		}

		void copy() {
			FILE *fp = fdopen(lib_fd, "r");
			size_t i = fread(fun, 1, lib_size, fp);
			if (i != lib_size) {
				perror(("reading lib failed, code: "s + std::to_string(i)).c_str());
				exit(EXIT_FAILURE);
			}
		}

	public:
		explicit functor(int lib_fd) : lib_fd(lib_fd) {
			struct stat s{};
			int status = fstat(lib_fd, &s);
			lib_size = s.st_size;

			fun = mmap(nullptr, lib_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (fun == MAP_FAILED) {
				perror("mmap failed");
				exit(EXIT_FAILURE);
			}

			copy();

			if (mprotect(fun, lib_size, PROT_READ | PROT_EXEC) == -1) {
				free_map();
				perror("mprotect failed");
				exit(EXIT_FAILURE);
			}
		}

		virtual ~functor() {
			free_map();
		}

		template<typename... Args>
		void operator()(Args &&... args) {
			int entity = 0;
			int start_pos = 470;

			try {
				F *pF = reinterpret_cast<F *>((char *) fun + start_pos);
				pF(std::forward<Args>(args)...);
			} catch (...) {
				printf("ERROR");
			}
		}
	};

	template<typename F>
	std::function<F> jit(std::string_view name, std::string_view call_view, std::string_view code) {
		auto lib_fd = compile(name, call_view, code);
		functor<F> f(lib_fd);
		return f;
	}
}

#define JIT_DECLARATION(name, body) std::string_view body_of_ ##name = #body; body

#define JIT_CALL(name, call, ...) \
    static std::function<decltype(call)> __##name##_handle; \
    ( \
            (__##name##_handle) \
    ? \
        __##name##_handle( __VA_ARGS__ ) \
    : \
        (__##name##_handle = rd_jit::jit<decltype(call)>(#name, #call, body_of_##name), \
        __##name##_handle( __VA_ARGS__ )) \
        )

#endif //OS_JIT_JIT_H
