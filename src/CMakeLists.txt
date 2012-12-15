include_directories(../include/cppauparser/)
aux_source_directory(./ lib_src)
add_library(cppauparser ${lib_src})

aux_source_directory(../include/cppauparser/ lib_inc)
install (TARGETS cppauparser DESTINATION lib)
install (DIRECTORY ../include/cppauparser/ DESTINATION include/cppauparser
         FILES_MATCHING PATTERN "*.h")
