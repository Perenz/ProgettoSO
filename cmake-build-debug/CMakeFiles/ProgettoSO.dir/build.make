# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2018.2.6\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2018.2.6\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "C:\ProgrammiUni\CLion Projects\ProgettoSO"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "C:\ProgrammiUni\CLion Projects\ProgettoSO\cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/ProgettoSO.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ProgettoSO.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ProgettoSO.dir/flags.make

CMakeFiles/ProgettoSO.dir/main.c.obj: CMakeFiles/ProgettoSO.dir/flags.make
CMakeFiles/ProgettoSO.dir/main.c.obj: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="C:\ProgrammiUni\CLion Projects\ProgettoSO\cmake-build-debug\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/ProgettoSO.dir/main.c.obj"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\ProgettoSO.dir\main.c.obj   -c "C:\ProgrammiUni\CLion Projects\ProgettoSO\main.c"

CMakeFiles/ProgettoSO.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ProgettoSO.dir/main.c.i"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "C:\ProgrammiUni\CLion Projects\ProgettoSO\main.c" > CMakeFiles\ProgettoSO.dir\main.c.i

CMakeFiles/ProgettoSO.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ProgettoSO.dir/main.c.s"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "C:\ProgrammiUni\CLion Projects\ProgettoSO\main.c" -o CMakeFiles\ProgettoSO.dir\main.c.s

# Object files for target ProgettoSO
ProgettoSO_OBJECTS = \
"CMakeFiles/ProgettoSO.dir/main.c.obj"

# External object files for target ProgettoSO
ProgettoSO_EXTERNAL_OBJECTS =

ProgettoSO.exe: CMakeFiles/ProgettoSO.dir/main.c.obj
ProgettoSO.exe: CMakeFiles/ProgettoSO.dir/build.make
ProgettoSO.exe: CMakeFiles/ProgettoSO.dir/linklibs.rsp
ProgettoSO.exe: CMakeFiles/ProgettoSO.dir/objects1.rsp
ProgettoSO.exe: CMakeFiles/ProgettoSO.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="C:\ProgrammiUni\CLion Projects\ProgettoSO\cmake-build-debug\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ProgettoSO.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\ProgettoSO.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ProgettoSO.dir/build: ProgettoSO.exe

.PHONY : CMakeFiles/ProgettoSO.dir/build

CMakeFiles/ProgettoSO.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\ProgettoSO.dir\cmake_clean.cmake
.PHONY : CMakeFiles/ProgettoSO.dir/clean

CMakeFiles/ProgettoSO.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "C:\ProgrammiUni\CLion Projects\ProgettoSO" "C:\ProgrammiUni\CLion Projects\ProgettoSO" "C:\ProgrammiUni\CLion Projects\ProgettoSO\cmake-build-debug" "C:\ProgrammiUni\CLion Projects\ProgettoSO\cmake-build-debug" "C:\ProgrammiUni\CLion Projects\ProgettoSO\cmake-build-debug\CMakeFiles\ProgettoSO.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/ProgettoSO.dir/depend

