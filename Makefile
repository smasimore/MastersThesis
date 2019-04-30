CC=g++

### COMMON VARS ###
SRC = $(wildcard src/*.c*)
INCLUDE = -Iinclude/
# Compile for c++11 and ignore c++14 compatibility warnings
FLAGS = -Wall -std=c++11 -Wno-c++14-compat

### CPPUTEST VARS ###
CPPUTEST_HOME = libs/cpputest
TEST_INCLUDE += -I$(CPPUTEST_HOME)/include -Itests/include
TEST_LIBRARIES += -L$(CPPUTEST_HOME)/cpputest_build/lib -lCppUTest -lCppUTestExt
# Grab all .cpp files from tests dir
TEST_SRC = $(wildcard tests/src/*.cpp)

fsw_test:
	@# Only make build folder if doesn't already exist.
	mkdir -p build
	@# Compile & link
	$(CC) -o build/$@ $(FLAGS) $(INCLUDE) $(SRC) $(TEST_SRC) $(TEST_INCLUDE)   \
		$(TEST_LIBRARIES)

# Remove binaries
clean:
	rm fsw_test
