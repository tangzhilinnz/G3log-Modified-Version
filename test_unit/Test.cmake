# g3log is a KjellKod Logger
# 2015 @author Kjell Hedström, hedstrom@kjellkod.cc 
# ==================================================================
# 2015 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own
#    risk and comes  with no warranties.
#
# This code is yours to share, use and modify with no strings attached
#   and no restrictions or obligations.
# ===================================================================


   # ============================================================================
   # TEST OPTIONS: Turn OFF the ones that is of no interest to you
   # ---- by default all is OFF: except 'g3log-FATAL-example -----
   # ---- the reason for this is that
   # ----- 1) the performance tests were only thoroughly tested on Ubuntu, not windows-
   #           (g3log windows/linux, but Google's glog only on linux)
   #
   #       2) The unit test were tested windows/linux
   # ============================================================================


   # Unit test for g3log  (cmake -DADD_G3LOG_UNIT_TEST=ON ..)
   option (ADD_G3LOG_UNIT_TEST "g3log unit tests" OFF)


   # 4. create the unit tests for g3log --- ONLY TESTED THE UNIT TEST ON LINUX
   # =========================
   IF (ADD_G3LOG_UNIT_TEST)
      # Download and unpack googletest at configure time
      # CMAKE_COMMAND: This is the full path to the CMake executable cmake which
      # is useful from custom commands that want to use the cmake -E option for
      # portable system commands. (e.g. /usr/local/bin/cmake)
      # CMAKE_GENERATOR: The name of the generator that is being used to generate
      # the build files. (e.g. Unix Makefiles, Ninja, etc.)
      # The value of this variable should never be modified by project code. 
      # A generator may be selected via the cmake -G option, interactively in
      # cmake-gui, or via the CMAKE_GENERATOR environment variable.
      configure_file(CMakeLists.txt.in
            googletest-download/CMakeLists.txt)
      execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
      execute_process(COMMAND ${CMAKE_COMMAND} --build .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

      # Prevent GoogleTest from overriding our compiler/linker options
      # when building with Visual Studio
      set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

      # Add googletest directly to our build. This adds
      # the following targets: gtest, gtest_main, gmock
      # and gmock_main
      # add_subdirectory(source_dir [binary_dir])
      # Adds a subdirectory to the build. The source_dir specifies the directory
      # in which the source CMakeLists.txt and code files are located. If it is 
      # a relative path it will be evaluated with respect to the current directory
      # (the typical usage), but it may also be an absolute path.
      # The binary_dir specifies the directory in which to place the output files. 
      # If it is a relative path it will be evaluated with respect to the current
      # output directory, but it may also be an absolute path.
      add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
            ${CMAKE_BINARY_DIR}/googletest-build)

      # The gtest/gmock targets carry header search path
      # dependencies automatically when using CMake 2.8.11 or
      # later. Otherwise we have to add them here ourselves.
      if (CMAKE_VERSION VERSION_LESS 2.8.11)
         include_directories("${gtest_SOURCE_DIR}/include"
                             "${gmock_SOURCE_DIR}/include")
      endif()

      # enable_testing(): This command should be in the source directory root 
      # because ctest expects to find a test file in the build directory root.
      # This command is automatically invoked when the CTest module is included,
      # except if the BUILD_TESTING option is turned off.
      enable_testing()

      set(DIR_UNIT_TEST ${g3log_SOURCE_DIR}/test_unit)
      message( STATUS "-DADD_G3LOG_UNIT_TEST=ON" )  

     # obs see this: http://stackoverflow.com/questions/9589192/how-do-i-change-the-number-of-template-arguments-supported-by-msvcs-stdtupl
     # and this: http://stackoverflow.com/questions/2257464/google-test-and-visual-studio-2010-rc
  

     IF (MSVC OR MINGW)  
        SET(OS_SPECIFIC_TEST test_crashhandler_windows)
     ENDIF(MSVC OR MINGW)

     SET(tests_to_run
            test_message 
            test_filechange 
            test_io 
            test_cpp_future_concepts 
            test_concept_sink 
            test_sink
            test_rotate_sink
            test_filter_sink
            ${OS_SPECIFIC_TEST}
        )
     SET(helper ${DIR_UNIT_TEST}/testing_helpers.h ${DIR_UNIT_TEST}/testing_helpers.cpp)
     SET(rotate_helper ${DIR_UNIT_TEST}/test_rotate_helper.h ${DIR_UNIT_TEST}/test_rotate_helper.cpp)
     include_directories(${DIR_UNIT_TEST})

     FOREACH(test ${tests_to_run} )
        SET(all_tests  ${all_tests} ${DIR_UNIT_TEST}/${test}.cpp )
        IF(${test} STREQUAL "test_filechange")
           add_executable(test_filechange ${DIR_UNIT_TEST}/${test}.cpp ${helper})
        ELSEIF(${test} STREQUAL "test_rotate_sink" OR ${test} STREQUAL "test_filter_sink")
           add_executable(${test} ${g3log_SOURCE_DIR}/test_main/test_main.cpp ${DIR_UNIT_TEST}/${test}.cpp ${rotate_helper})
        ELSE()
           add_executable(${test} ${g3log_SOURCE_DIR}/test_main/test_main.cpp ${DIR_UNIT_TEST}/${test}.cpp ${helper})
        ENDIF()

        set_target_properties(${test} PROPERTIES COMPILE_DEFINITIONS "GTEST_HAS_TR1_TUPLE=0")
        set_target_properties(${test} PROPERTIES COMPILE_DEFINITIONS "GTEST_HAS_RTTI=0")
        IF( NOT(MSVC))
           # Include flags for directories marked as SYSTEM are now moved after
           # non-system directories. The -isystem flag does this automatically,
           # so moving them explicitly to the end makes the behavior consistent
           # on compilers that do not have any -isystem flag.
           set_target_properties(${test} PROPERTIES COMPILE_FLAGS "-isystem -pthread ")
        ENDIF( NOT(MSVC)) 
        target_link_libraries(${test} g3log gtest_main)
        # add_test(<name> <command> [<arg>...])
        # Add a test called <name> with the given command-line <command> to the 
        # project to be run by ctest. <name> may contain arbitrary characters,
        # expressed as a Quoted Argument or Bracket Argument if necessary. If 
        # <command> specifies an executable target (created by add_executable()) 
        # it will automatically be replaced by the location of the executable 
        # created at build time.
        add_test( ${test} ${test} )
     ENDFOREACH(test)
   
    #
    # Test for Linux, runtime loading of dynamic libraries
    #     
    IF (NOT WIN32 AND NOT ("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang") AND G3_SHARED_LIB)
       add_library(tester_sharedlib SHARED ${DIR_UNIT_TEST}/tester_sharedlib.h ${DIR_UNIT_TEST}/tester_sharedlib.cpp)
       target_link_libraries(tester_sharedlib ${G3LOG_LIBRARY})

       add_executable(test_dynamic_loaded_shared_lib ${g3log_SOURCE_DIR}/test_main/test_main.cpp ${DIR_UNIT_TEST}/test_linux_dynamic_loaded_sharedlib.cpp)
       set_target_properties(test_dynamic_loaded_shared_lib PROPERTIES COMPILE_DEFINITIONS "GTEST_HAS_TR1_TUPLE=0")
       set_target_properties(test_dynamic_loaded_shared_lib PROPERTIES COMPILE_DEFINITIONS "GTEST_HAS_RTTI=0")
       # -ldl - dynamic linking library (libdl.so.1)
       # #include <dlfcn.h>
       # dlopen(), dlclose(), dlerror(), dlsym(), the Base Definitions volume of IEEE Std 1003.1-2001, <dlfcn.h>
       target_link_libraries(test_dynamic_loaded_shared_lib  ${G3LOG_LIBRARY} -ldl gtest_main)
    ENDIF()
ELSE() 
  message( STATUS "-DADD_G3LOG_UNIT_TEST=OFF" ) 
ENDIF (ADD_G3LOG_UNIT_TEST)
