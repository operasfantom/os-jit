#include "jit.h"

#include <iostream>
#include <fstream>
#include <bits/dlfcn.h>
#include <dlfcn.h>
#include <error.h>
#include <sys/stat.h>
#include <link.h>
#include <fcntl.h>

namespace rd_jit {
	int compile(std::string_view name_view, std::string_view call_view, std::string_view code_view) {
		std::cerr << "NAME:" << name_view << std::endl;
		std::cerr << "CALL:" << call_view << std::endl;
		std::cerr << "CODE:" << code_view << std::endl;

		std::string name(name_view);
		std::string tmp_folder = "/tmp/jit/";

		mkdir(tmp_folder.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);

		std::string source_path = tmp_folder + name + ".cpp";
		std::ofstream out(source_path);

		out << code_view << std::endl;
		out << "template void " << call_view << "();";

		out.close();

		std::string library_path = tmp_folder + name + ".so";
		std::string command =
				"cc " + source_path + " -o " + library_path + " -nostdlib -shared -fPIC -fno-plt -fno-pic";
		int code = system(command.c_str());
		if (WIFEXITED(code) == 0) {
			perror(("compilation of " + name + " failed").c_str());
			exit(EXIT_FAILURE);
		}
		int fd_lib = open(library_path.c_str(), O_RDONLY);

		if (fd_lib == -1) {
			perror("openning shared library failed");
			exit(EXIT_FAILURE);
		}
		return fd_lib;
	}
}