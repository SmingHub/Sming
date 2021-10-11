#
# cxx
#

ifndef CONFIG_COMPILER_CXX_EXCEPTIONS
# If exceptions are disabled, ensure our fatal exception
# hooks are preferentially linked over libstdc++ which
# has full exception support
SDK_WRAP_SYMBOLS += \
        _Unwind_SetEnableExceptionFdeSorting \
        __register_frame_info_bases \
        __register_frame_info \
        __register_frame \
        __register_frame_info_table_bases \
        __register_frame_info_table \
        __register_frame_table \
        __deregister_frame_info_bases \
        __deregister_frame_info \
        _Unwind_Find_FDE \
        _Unwind_GetGR \
        _Unwind_GetCFA \
        _Unwind_GetIP \
        _Unwind_GetIPInfo \
        _Unwind_GetRegionStart \
        _Unwind_GetDataRelBase \
        _Unwind_GetTextRelBase \
        _Unwind_SetIP \
        _Unwind_SetGR \
        _Unwind_GetLanguageSpecificData \
        _Unwind_FindEnclosingFunction \
        _Unwind_Resume \
        _Unwind_RaiseException \
        _Unwind_DeleteException \
        _Unwind_ForcedUnwind \
        _Unwind_Resume_or_Rethrow \
        _Unwind_Backtrace \
        __cxa_call_unexpected \
        __gxx_personality_v0

SDK_UNDEF_SYMBOLS += __cxx_fatal_exception
endif

SDK_UNDEF_SYMBOLS += __cxa_guard_dummy

# Force libpthread to appear later than libstdc++ in link line since libstdc++ depends on libpthread.
# Furthermore, force libcxx to appear later than libgcc because some libgcc unwind code is wrapped, if C++
# exceptions are disabled. libcxx (this component) provides the unwind code wrappers.
# This is to prevent linking of libgcc's unwind code which considerably increases the binary size.

# idf_component_get_property(pthread pthread COMPONENT_LIB)
# idf_component_get_property(cxx cxx COMPONENT_LIB)
# add_library(stdcpp_pthread INTERFACE)
# target_link_libraries(stdcpp_pthread INTERFACE stdc++ $<TARGET_FILE:${pthread}>)
# target_link_libraries(${COMPONENT_LIB} PUBLIC stdcpp_pthread)
# add_library(libgcc_cxx INTERFACE)
# target_link_libraries(libgcc_cxx INTERFACE gcc $<TARGET_FILE:${cxx}>)
# target_link_libraries(${COMPONENT_LIB} PUBLIC libgcc_cxx)
