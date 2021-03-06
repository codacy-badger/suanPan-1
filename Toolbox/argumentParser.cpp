////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017-2021 Theodore Chang
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#include "argumentParser.h"
#include <Step/Bead.h>
#include <Toolbox/Converter.h>
#include <Toolbox/commandParser.h>
#include <Toolbox/utility.h>
#include <UnitTest/CatchTest.h>
#include <iomanip>

using std::ifstream;
using std::ofstream;
using std::string;

void argument_parser(const int argc, char** argv) {
	string input_file_name, output_file_name;
	ofstream output_file;
	const auto buffer_backup = cout.rdbuf();

	wall_clock T;
	T.tic();

	SUANPAN_EXE = argv[0];

	if(argc > 1) {
		auto strip = false, convert = false;

		for(auto I = 1; I < argc; ++I) {
			if(is_equal(argv[I], "-v") || is_equal(argv[I], "--version")) print_version();
			else if(is_equal(argv[I], "-h") || is_equal(argv[I], "--help")) print_helper();
			else if(is_equal(argv[I], "-t") || is_equal(argv[I], "--test")) test_mode();
			else if(is_equal(argv[I], "-f") || is_equal(argv[I], "--file")) input_file_name = argv[++I];
			else if(is_equal(argv[I], "-o") || is_equal(argv[I], "--output")) output_file_name = argv[++I];
			else if(is_equal(argv[I], "-np") || is_equal(argv[I], "--noprint")) SUANPAN_PRINT = false;
			else if(is_equal(argv[I], "-s") || is_equal(argv[I], "--strip")) {
				strip = true;
				convert = false;
			} else if(is_equal(argv[I], "-c") || is_equal(argv[I], "--convert")) {
				convert = true;
				strip = false;
			} else if(is_equal(argv[I], "-ctest") || is_equal(argv[I], "--catch2test")) {
				catchtest_main(argc, argv);
				return;
			}
		}

		if(strip || convert) {
			if(input_file_name.empty()) return;

			if(output_file_name.empty()) {
				output_file_name = input_file_name;

				auto found = output_file_name.rfind(".inp");

				if(std::string::npos == found) found = output_file_name.rfind(".INP");

				if(std::string::npos != found) output_file_name.erase(found, 4);

				output_file_name += strip ? "_out.inp" : "_out.supan";
			}

			for(auto& I : output_file_name) if(I == '\\') I = '/';

			return convert ? convert_mode(input_file_name, output_file_name) : strip_mode(input_file_name, output_file_name);
		}

		if(!output_file_name.empty()) {
			output_file.open(output_file_name);
			if(output_file.is_open()) cout.rdbuf(output_file.rdbuf());
			else suanpan_error("argumentParser() cannot open the output file.\n");
		}

		if(!input_file_name.empty()) {
			print_header();
			const auto model = make_shared<Bead>();
			if(process_file(model, input_file_name.c_str()) != SUANPAN_EXIT) {
				if(output_file.is_open()) {
					cout.rdbuf(buffer_backup);
					print_header();
				}
				cli_mode(model);
			}
		}
	} else {
		print_header();
		const auto model = make_shared<Bead>();
		cli_mode(model);
	}

	suanpan_print("\nRun for %.4F seconds.\n", T.toc());
}

void print_header() {
	suanpan_info("+--------------------------------------------------+\n");
	suanpan_info("|   __        __         suanPan is an open source |\n");
	suanpan_info("|  /  \\      |  \\           FEM framework (%u-bit) |\n", SUANPAN_ARCH);
	suanpan_info("|  \\__       |__/  __   __      %s (%u.%u.%u) |\n", SUANPAN_CODE, SUANPAN_MAJOR, SUANPAN_MINOR, SUANPAN_PATCH);
	suanpan_info("|     \\ |  | |    |  \\ |  |      maintained by tlc |\n");
	suanpan_info("|  \\__/ |__| |    |__X |  |    all rights reserved |\n");
	suanpan_info("|                           10.5281/zenodo.1285221 |\n");
	suanpan_info("+--------------------------------------------------+\n");
#if 1
	suanpan_info("|  https://github.com/TLCFEM/suanPan               |\n");
	suanpan_info("|  https://github.com/TLCFEM/suanPan-manual        |\n");
	suanpan_info("+--------------------------------------------------+\n");
	suanpan_info("|  https://gitter.im/suanPan-dev/community         |\n");
#else
    vector<const char*> POOL;
    POOL.reserve(10);
    POOL.emplace_back(u8"\U0001F308");
    POOL.emplace_back(u8"\U0001F30F");
    POOL.emplace_back(u8"\U0001F3A7");
    POOL.emplace_back(u8"\U0001F3B1");
    POOL.emplace_back(u8"\U0001F479");
    POOL.emplace_back(u8"\U0001F4BB");
    POOL.emplace_back(u8"\U0001F50B");
    POOL.emplace_back(u8"\U0001F514");
    POOL.emplace_back(u8"\U0001F680");
    POOL.emplace_back(u8"\U0001F9E9");
    srand(static_cast<unsigned>(time(nullptr)));
    suanpan_info("|  %-6shttp://doi.org/10.5281/zenodo.1285221       |\n", POOL[std::rand() % POOL.size()]);
    suanpan_info("+--------------------------------------------------+\n");
    suanpan_info("|  %-6shttps://github.com/TLCFEM/suanPan           |\n", u8"\U0001F9EE");
    suanpan_info("|  %-6shttps://github.com/TLCFEM/suanPan-manual    |\n", u8"\U0001F4DA");
#endif
	suanpan_info("+--------------------------------------------------+\n\n");
}

void print_version() {
	suanpan_info("Copyright (C) 2017-2021 Theodore Chang\n\n");
	suanpan_info("This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\n");
	suanpan_info("This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\n");
	suanpan_info("You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n\n");
	suanpan_info("suanPan is an open source FEM framework.\n");
	suanpan_info("    The binary is compiled on %s\n", __DATE__);
	suanpan_info("    The source code of suanPan is hosted on GitHub. https://tlcfem.github.io/suanPan/\n");
	suanpan_info("    The documentation is hosted on GitBook and readthedocs. https://tlcfem.gitbook.io/suanpan-manual/ and https://suanpan-manual.readthedocs.io/\n");
#ifdef SUANPAN_MKL
	suanpan_info("    The linear algebra support is provided by Armadillo with Intel MKL. http://arma.sourceforge.net/\n");
#else
	suanpan_info("    The linear algebra support is provided by Armadillo. http://arma.sourceforge.net/\n");
#endif
#ifdef SUANPAN_MT
	suanpan_info("    The parallelisation support is implemented via TBB library. https://github.com/oneapi-src/oneTBB\n");
#endif
#ifdef SUANPAN_VTK
	suanpan_info("    The visualisation support is implemented via VTK library. https://vtk.org/\n");
#endif
	suanpan_info("\nPlease join gitter for any feedbacks. https://gitter.im/suanPan-dev/community\n");
	suanpan_info("\n\n[From Wikipedia] Alpha Crucis is a multiple star system located 321 light years from the Sun in the constellation of Crux and part of the asterism known as the Southern Cross.\n\n");
}

void print_helper() {
	suanpan_info("Available Parameters:\n");
	suanpan_info("\t-%-10s  --%-20s%s\n", "v", "version", "check version information");
	suanpan_info("\t-%-10s  --%-20s%s\n", "h", "help", "print this helper");
	suanpan_info("\t-%-10s  --%-20s%s\n", "s", "strip", "strip comments out in given ABAQUS input file");
	// suanpan_info("\t-%-10s  --%-20s%s\n", "c", "convert", "partially convert ABAQUS input file into suanPan model script");
	suanpan_info("\t-%-10s  --%-20s%s\n", "np", "noprint", "suppress most console output");
	suanpan_info("\t-%-10s  --%-20s%s\n", "f", "file", "process model file");
	suanpan_info("\t-%-10s  --%-20s%s\n", "o", "output", "set output file for logging");
	suanpan_info("\n");
}

void cli_mode(const shared_ptr<Bead>& model) {
	string all_line;
	while(true) {
		string command_line;
		suanpan_info("suanPan ~<> ");
		getline(std::cin, command_line);
		if(!command_line.empty() && command_line[0] != '#' && command_line[0] != '!') {
			const auto if_comment = command_line.find('!');
			if(string::npos != if_comment) command_line.erase(if_comment);
			for(auto& c : command_line) if(',' == c || '\t' == c || '\r' == c || '\n' == c) c = ' ';
			while(*command_line.crbegin() == ' ') command_line.pop_back();
			if(*command_line.crbegin() == '\\') {
				command_line.back() = ' ';
				all_line.append(command_line);
			} else {
				all_line.append(command_line);
				istringstream tmp_str(all_line);
				if(process_command(model, tmp_str) == SUANPAN_EXIT) return;
				all_line.clear();
			}
		}
	}
}

void strip_mode(const string& input_file_name, const string& output_file_name) {
	ifstream input_file(input_file_name);

	if(!input_file.is_open()) {
		suanpan_error("fail to open %s\n", input_file_name.c_str());
		return;
	}

	ofstream output_file(output_file_name);

	if(!output_file.is_open()) {
		suanpan_error("fail to open %s\n", output_file_name.c_str());
		return;
	}

	output_file.setf(std::ios::scientific);
	output_file << std::setprecision(3);

	string line;

	while(std::getline(input_file, line)) {
		if(line.empty() || if_contain(line, "**")) continue;

		for(auto& I : line) I = static_cast<char>(std::tolower(static_cast<int>(I)));

		output_file << line << '\n';
	}
}

void convert_mode(const string& input_file_name, const string& output_file_name) {
	ifstream input_file(input_file_name);

	if(!input_file.is_open()) {
		suanpan_error("fail to open %s\n", input_file_name.c_str());
		return;
	}

	ofstream output_file(output_file_name);

	if(!output_file.is_open()) {
		suanpan_error("fail to open %s\n", output_file_name.c_str());
		return;
	}

	const auto pos = output_file_name.find_last_of('/');

	Converter abaqus_converter(string::npos == pos ? "" : output_file_name.substr(0, pos + 1));

	abaqus_converter.process(input_file, output_file);
}
