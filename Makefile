CC=g++

### COMMON VARS ###
SRC = $(wildcard src/*.c)
INCLUDE = -Iinclude/

### CPPUTEST VARS ###
CPPUTEST_HOME = ../libs/cpputest
CPPFLAGS += -I$(CPPUTEST_HOME)/include
LD_LIBRARIES += -L$(CPPUTEST_HOME)/cpputest_build/lib -lCppUTest -lCppUTestExt
# Grab all .cpp files from tests dir
TEST_FILES = $(wildcard tests/*.cpp)

fsw_test:
	# Only make build folder if doesn't already exist.
	mkdir -p build
	# Compile & link
	$(CC) -o build/$@ $(INCLUDE) $(SRC) $(TEST_FILES) $(CPPFLAGS) $(LD_LIBRARIES)

# Remove binaries
clean:
	rm fsw_test
