# Install script for directory: C:/Users/jason m/Desktop/log/g3log-master

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/jason m/Desktop/log/g3log-master/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibrariesx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/g3log.lib")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibrariesx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/g3log.dll")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xheadersx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/g3log" TYPE FILE FILES
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/active.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/atomicbool.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/crashhandler.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/filesink.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/future.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/g3log.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/logcapture.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/loglevels.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/logmessage.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/logworker.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/moveoncopy.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/shared_queue.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/sink.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/sinkhandle.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/sinkwrapper.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/stacktrace_windows.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/stlpatch_future.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/src/g3log/time.hpp"
    "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/include/g3log/generated_definitions.hpp"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/g3log/g3logTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/g3log/g3logTargets.cmake"
         "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/CMakeFiles/Export/lib/cmake/g3log/g3logTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/g3log/g3logTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/g3log/g3logTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/g3log" TYPE FILE FILES "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/CMakeFiles/Export/lib/cmake/g3log/g3logTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/g3log" TYPE FILE FILES "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/CMakeFiles/Export/lib/cmake/g3log/g3logTargets-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/g3log" TYPE FILE FILES "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/g3logConfig.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/jason m/Desktop/log/g3log-master/out/build/x64-Debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
