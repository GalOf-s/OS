# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/dorelby/Downloads/CLion-2021.3.3/clion-2021.3.3/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/dorelby/Downloads/CLion-2021.3.3/clion-2021.3.3/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dorelby/CLionProjects/OS/OS/ex2/uthreads

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/uthreads.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/uthreads.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/uthreads.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/uthreads.dir/flags.make

CMakeFiles/uthreads.dir/uthreads.cpp.o: CMakeFiles/uthreads.dir/flags.make
CMakeFiles/uthreads.dir/uthreads.cpp.o: ../uthreads.cpp
CMakeFiles/uthreads.dir/uthreads.cpp.o: CMakeFiles/uthreads.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/uthreads.dir/uthreads.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/uthreads.dir/uthreads.cpp.o -MF CMakeFiles/uthreads.dir/uthreads.cpp.o.d -o CMakeFiles/uthreads.dir/uthreads.cpp.o -c /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/uthreads.cpp

CMakeFiles/uthreads.dir/uthreads.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uthreads.dir/uthreads.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/uthreads.cpp > CMakeFiles/uthreads.dir/uthreads.cpp.i

CMakeFiles/uthreads.dir/uthreads.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uthreads.dir/uthreads.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/uthreads.cpp -o CMakeFiles/uthreads.dir/uthreads.cpp.s

CMakeFiles/uthreads.dir/Scheduler.cpp.o: CMakeFiles/uthreads.dir/flags.make
CMakeFiles/uthreads.dir/Scheduler.cpp.o: ../Scheduler.cpp
CMakeFiles/uthreads.dir/Scheduler.cpp.o: CMakeFiles/uthreads.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/uthreads.dir/Scheduler.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/uthreads.dir/Scheduler.cpp.o -MF CMakeFiles/uthreads.dir/Scheduler.cpp.o.d -o CMakeFiles/uthreads.dir/Scheduler.cpp.o -c /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/Scheduler.cpp

CMakeFiles/uthreads.dir/Scheduler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uthreads.dir/Scheduler.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/Scheduler.cpp > CMakeFiles/uthreads.dir/Scheduler.cpp.i

CMakeFiles/uthreads.dir/Scheduler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uthreads.dir/Scheduler.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/Scheduler.cpp -o CMakeFiles/uthreads.dir/Scheduler.cpp.s

CMakeFiles/uthreads.dir/Thread.cpp.o: CMakeFiles/uthreads.dir/flags.make
CMakeFiles/uthreads.dir/Thread.cpp.o: ../Thread.cpp
CMakeFiles/uthreads.dir/Thread.cpp.o: CMakeFiles/uthreads.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/uthreads.dir/Thread.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/uthreads.dir/Thread.cpp.o -MF CMakeFiles/uthreads.dir/Thread.cpp.o.d -o CMakeFiles/uthreads.dir/Thread.cpp.o -c /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/Thread.cpp

CMakeFiles/uthreads.dir/Thread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uthreads.dir/Thread.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/Thread.cpp > CMakeFiles/uthreads.dir/Thread.cpp.i

CMakeFiles/uthreads.dir/Thread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uthreads.dir/Thread.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/Thread.cpp -o CMakeFiles/uthreads.dir/Thread.cpp.s

CMakeFiles/uthreads.dir/ThreadManager.cpp.o: CMakeFiles/uthreads.dir/flags.make
CMakeFiles/uthreads.dir/ThreadManager.cpp.o: ../ThreadManager.cpp
CMakeFiles/uthreads.dir/ThreadManager.cpp.o: CMakeFiles/uthreads.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/uthreads.dir/ThreadManager.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/uthreads.dir/ThreadManager.cpp.o -MF CMakeFiles/uthreads.dir/ThreadManager.cpp.o.d -o CMakeFiles/uthreads.dir/ThreadManager.cpp.o -c /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/ThreadManager.cpp

CMakeFiles/uthreads.dir/ThreadManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uthreads.dir/ThreadManager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/ThreadManager.cpp > CMakeFiles/uthreads.dir/ThreadManager.cpp.i

CMakeFiles/uthreads.dir/ThreadManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uthreads.dir/ThreadManager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/ThreadManager.cpp -o CMakeFiles/uthreads.dir/ThreadManager.cpp.s

# Object files for target uthreads
uthreads_OBJECTS = \
"CMakeFiles/uthreads.dir/uthreads.cpp.o" \
"CMakeFiles/uthreads.dir/Scheduler.cpp.o" \
"CMakeFiles/uthreads.dir/Thread.cpp.o" \
"CMakeFiles/uthreads.dir/ThreadManager.cpp.o"

# External object files for target uthreads
uthreads_EXTERNAL_OBJECTS =

libuthreads.a: CMakeFiles/uthreads.dir/uthreads.cpp.o
libuthreads.a: CMakeFiles/uthreads.dir/Scheduler.cpp.o
libuthreads.a: CMakeFiles/uthreads.dir/Thread.cpp.o
libuthreads.a: CMakeFiles/uthreads.dir/ThreadManager.cpp.o
libuthreads.a: CMakeFiles/uthreads.dir/build.make
libuthreads.a: CMakeFiles/uthreads.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library libuthreads.a"
	$(CMAKE_COMMAND) -P CMakeFiles/uthreads.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uthreads.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/uthreads.dir/build: libuthreads.a
.PHONY : CMakeFiles/uthreads.dir/build

CMakeFiles/uthreads.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/uthreads.dir/cmake_clean.cmake
.PHONY : CMakeFiles/uthreads.dir/clean

CMakeFiles/uthreads.dir/depend:
	cd /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dorelby/CLionProjects/OS/OS/ex2/uthreads /home/dorelby/CLionProjects/OS/OS/ex2/uthreads /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug /home/dorelby/CLionProjects/OS/OS/ex2/uthreads/cmake-build-debug/CMakeFiles/uthreads.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/uthreads.dir/depend

