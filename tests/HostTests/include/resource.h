#pragma once

#include <FlashString/String.hpp>

namespace Resource
{
DECLARE_FSTR(abstract_txt)
DECLARE_FSTR(test_json)
DECLARE_FSTR(README_md)

// Templating
DECLARE_FSTR(unit_testing_rst) // Unmodified file
DECLARE_FSTR(ut_template1_in_rst)
DECLARE_FSTR(ut_template1_out1_rst) // emit_contents = true

// Multipart streams
DECLARE_FSTR(image_png)
DECLARE_FSTR(multipart_result)

} // namespace Resource
