# FlightSoftware
All code flying on the rocket must be in this repository.

## Running Tests
sh ./RunTests.sh

## Cpputest
CppUTest is used to create unit & integration tests for flight software. 
[Cpputest Manual](https://cpputest.github.io/manual.html)

### Building cpputest library (only needs to be done once per machine)
1. sudo apt install autoconf libtool make
2. cd libs/cpputest/cpputest_build
3. autoreconf .. -i
4. ../configure
5. make
6. make check

### Creating a test
1. Add a test file in fsw/tests/
2. Run sh ./RunTests.sh to verify your test is now running as part of the FSW test suite
