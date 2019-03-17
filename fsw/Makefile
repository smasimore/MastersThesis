CC=g++

# Cpputest vars
CPPUTEST_HOME = ../libs/cpputest
CPPFLAGS += -I$(CPPUTEST_HOME)/include
LD_LIBRARIES += -L$(CPPUTEST_HOME)/cpputest_build/lib -lCppUTest -lCppUTestExt
# Grab all .cpp files from tests dir
TEST_FILES = $(wildcard tests/*.cpp)

fsw_test:
	mkdir -p build
	$(CC) -o build/$@ $(TEST_FILES) $(CPPFLAGS) $(LD_LIBRARIES)

# Remove binaries
clean:
	rm fsw_test
