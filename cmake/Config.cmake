#
# standard compiler and linker settings
#

if (MSVC)
   set(CMAKE_CXX_FLAGS                "/EHsc /TP -W4")
   set(CMAKE_CXX_FLAGS_RELEASE        "-Ox -DNDEBUG")
   set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
   add_definitions( "/DWIN32 /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS /wd4710 /wd4514 /nologo" ) # disable 'inlining' warnings
   if(NOT((MSVC_VERSION GREATER 1600) OR (MSVC_VERSION EQUAL 1600)))
      message(FATAL_ERROR "Your C++ compiler (${CMAKE_CXX_COMPILER_ID}) ${MSVC_VERSION} does not support C++11.")
   endif ()
else ()
   set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
   set(CMAKE_CXX_FLAGS_RELEASE        "-DNDEBUG -04")
   set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
   set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
   if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
      execute_process(
         COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
      if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
         message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
      endif ()
   elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

      if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
         set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
      endif ()
   else ()
      message(FATAL_ERROR "Your C++ compiler (${CMAKE_CXX_COMPILER_ID}) is not configured")
   endif ()

endif ()
