# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lqf/cpp/sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lqf/cpp/sylar/build

# Include any dependencies generated for this target.
include CMakeFiles/tests_Timer.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/tests_Timer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tests_Timer.dir/flags.make

CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o: CMakeFiles/tests_Timer.dir/flags.make
CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o: ../tests/test_Timer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lqf/cpp/sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) -D__FILE__=\"tests/test_Timer.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o -c /home/lqf/cpp/sylar/tests/test_Timer.cpp

CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) -D__FILE__=\"tests/test_Timer.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lqf/cpp/sylar/tests/test_Timer.cpp > CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.i

CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) -D__FILE__=\"tests/test_Timer.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lqf/cpp/sylar/tests/test_Timer.cpp -o CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.s

CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.requires:

.PHONY : CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.requires

CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.provides: CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.requires
	$(MAKE) -f CMakeFiles/tests_Timer.dir/build.make CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.provides.build
.PHONY : CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.provides

CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.provides.build: CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o


# Object files for target tests_Timer
tests_Timer_OBJECTS = \
"CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o"

# External object files for target tests_Timer
tests_Timer_EXTERNAL_OBJECTS =

../bin/tests_Timer: CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o
../bin/tests_Timer: CMakeFiles/tests_Timer.dir/build.make
../bin/tests_Timer: ../lib/libsylar.so
../bin/tests_Timer: CMakeFiles/tests_Timer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lqf/cpp/sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/tests_Timer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tests_Timer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tests_Timer.dir/build: ../bin/tests_Timer

.PHONY : CMakeFiles/tests_Timer.dir/build

CMakeFiles/tests_Timer.dir/requires: CMakeFiles/tests_Timer.dir/tests/test_Timer.cpp.o.requires

.PHONY : CMakeFiles/tests_Timer.dir/requires

CMakeFiles/tests_Timer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tests_Timer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tests_Timer.dir/clean

CMakeFiles/tests_Timer.dir/depend:
	cd /home/lqf/cpp/sylar/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lqf/cpp/sylar /home/lqf/cpp/sylar /home/lqf/cpp/sylar/build /home/lqf/cpp/sylar/build /home/lqf/cpp/sylar/build/CMakeFiles/tests_Timer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tests_Timer.dir/depend

