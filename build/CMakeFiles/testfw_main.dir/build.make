# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/net/cremi/aperigno/PROJET PS/testfw"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/net/cremi/aperigno/PROJET PS/testfw/build"

# Include any dependencies generated for this target.
include CMakeFiles/testfw_main.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/testfw_main.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testfw_main.dir/flags.make

CMakeFiles/testfw_main.dir/testfw_main.c.o: CMakeFiles/testfw_main.dir/flags.make
CMakeFiles/testfw_main.dir/testfw_main.c.o: ../testfw_main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/net/cremi/aperigno/PROJET PS/testfw/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/testfw_main.dir/testfw_main.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/testfw_main.dir/testfw_main.c.o   -c "/net/cremi/aperigno/PROJET PS/testfw/testfw_main.c"

CMakeFiles/testfw_main.dir/testfw_main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/testfw_main.dir/testfw_main.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/net/cremi/aperigno/PROJET PS/testfw/testfw_main.c" > CMakeFiles/testfw_main.dir/testfw_main.c.i

CMakeFiles/testfw_main.dir/testfw_main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/testfw_main.dir/testfw_main.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/net/cremi/aperigno/PROJET PS/testfw/testfw_main.c" -o CMakeFiles/testfw_main.dir/testfw_main.c.s

CMakeFiles/testfw_main.dir/testfw_main.c.o.requires:

.PHONY : CMakeFiles/testfw_main.dir/testfw_main.c.o.requires

CMakeFiles/testfw_main.dir/testfw_main.c.o.provides: CMakeFiles/testfw_main.dir/testfw_main.c.o.requires
	$(MAKE) -f CMakeFiles/testfw_main.dir/build.make CMakeFiles/testfw_main.dir/testfw_main.c.o.provides.build
.PHONY : CMakeFiles/testfw_main.dir/testfw_main.c.o.provides

CMakeFiles/testfw_main.dir/testfw_main.c.o.provides.build: CMakeFiles/testfw_main.dir/testfw_main.c.o


# Object files for target testfw_main
testfw_main_OBJECTS = \
"CMakeFiles/testfw_main.dir/testfw_main.c.o"

# External object files for target testfw_main
testfw_main_EXTERNAL_OBJECTS =

libtestfw_main.a: CMakeFiles/testfw_main.dir/testfw_main.c.o
libtestfw_main.a: CMakeFiles/testfw_main.dir/build.make
libtestfw_main.a: CMakeFiles/testfw_main.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/net/cremi/aperigno/PROJET PS/testfw/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libtestfw_main.a"
	$(CMAKE_COMMAND) -P CMakeFiles/testfw_main.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testfw_main.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testfw_main.dir/build: libtestfw_main.a

.PHONY : CMakeFiles/testfw_main.dir/build

CMakeFiles/testfw_main.dir/requires: CMakeFiles/testfw_main.dir/testfw_main.c.o.requires

.PHONY : CMakeFiles/testfw_main.dir/requires

CMakeFiles/testfw_main.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testfw_main.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testfw_main.dir/clean

CMakeFiles/testfw_main.dir/depend:
	cd "/net/cremi/aperigno/PROJET PS/testfw/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/net/cremi/aperigno/PROJET PS/testfw" "/net/cremi/aperigno/PROJET PS/testfw" "/net/cremi/aperigno/PROJET PS/testfw/build" "/net/cremi/aperigno/PROJET PS/testfw/build" "/net/cremi/aperigno/PROJET PS/testfw/build/CMakeFiles/testfw_main.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/testfw_main.dir/depend

