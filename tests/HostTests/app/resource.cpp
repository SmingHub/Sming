#include <resource.h>

namespace Resource
{
IMPORT_FSTR(README_md, SMING_HOME "/../README.md");

#define RESOURCE(name, file) IMPORT_FSTR(name, PROJECT_DIR "/resource/" file)

RESOURCE(abstract_txt, "abstract.txt")
RESOURCE(test_json, "test.json")
RESOURCE(ut_template1_in_rst, "ut_template1.in.rst")
RESOURCE(ut_template1_out1_rst, "ut_template1.out1.rst")
//RESOURCE(ut_template2_in_rst, "ut_template2.in.rst")
//RESOURCE(ut_template2_out_rst, "ut_template2.in.rst")

} // namespace Resource
