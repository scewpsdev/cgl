#include "cgl/CGLCompiler.h"

#include "libtcc/libtcc.h"


char my_program[] =
"#include <tcclib.h>\n" /* include the "Simple libc header for TCC" */
"int main(int argc, char *argv[])\n"
"{\n"
"    printf(\"%s\\n\", \"Hello World!\\n\");\n"
"    return 0;\n"
"}\n";


int CGLCompiler::run(int argc, char* argv[])
{
	TCCState* tcc = tcc_new();

	//tcc_set_lib_path(tcc, "../../cgl/src/libtcc/");
	tcc_add_include_path(tcc, "C:\\Users\\faris\\Documents\\Dev\\tcc\\include\\");
	tcc_add_library_path(tcc, "C:\\Users\\faris\\Documents\\Dev\\tcc\\lib\\");
	tcc_add_library(tcc, "tcc1-64");

	tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);

	if (tcc_compile_string(tcc, my_program) == -1)
		return false;

	int result = tcc_run(tcc, argc, argv);

	tcc_delete(tcc);

	return result;
}
