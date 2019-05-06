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
#include <memory>

namespace rd_jit {
	using std::string_literals::operator ""s;

	int compile(std::string_view name_view, std::string_view call_view, std::string_view code_view);

	template<typename F>
	class functor {
	private:
		int lib_fd;
		size_t lib_size;

		std::shared_ptr<void> fun;

		void copy() {
			FILE *fp = fdopen(lib_fd, "r");
			size_t i = fread(fun.get(), 1, lib_size, fp);
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

			fun = std::shared_ptr<void>(mmap(nullptr, lib_size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0), [sz = lib_size](void *ptr) {
				munmap(ptr, sz);
			});
			if (fun.get() == MAP_FAILED) {
				perror("mmap failed");
				exit(EXIT_FAILURE);
			}

			copy();

			if (mprotect(fun.get(), lib_size, PROT_EXEC) == -1) {
				perror("mprotect failed");
				exit(EXIT_FAILURE);
			}
		}

		functor(functor const &other) = default;

		functor &operator=(functor const &) = default;

		functor(functor &&) noexcept = default;

		functor &operator=(functor &&) noexcept = default;

		~functor() = default;

		template<typename... Args>
		void operator()(Args &&... args) {
			int entity = 0;
			int start_pos = 72;//hard code

			try {
				void *ptr = (char *) fun.get() + start_pos;
				auto pF = reinterpret_cast<F*>(ptr);
				pF(std::forward<Args>(args)...);
			} catch (...) {
				printf("Runtime exception caught while executing functor");
			}
		}
	};

	template<typename F>
	std::function<F> jit(std::string_view name, std::string_view call_view, std::string_view code) {
		auto lib_fd = compile(name, call_view, code);
		functor<F> f(lib_fd);
		return {std::move(f)};
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
