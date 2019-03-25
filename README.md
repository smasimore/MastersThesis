# FlightSoftware
All code flying on the rocket must be in this repository.

## Cpputest
CppUTest is used to create unit & integration tests for flight software. 
[Cpputest Manual](https://cpputest.github.io/manual.html)

### Building cpputest library (only needs to be done once per machine)
1. cd libs/cpputest/cpputest_build
2. autoreconf .. -i
3. ../configure

### Creating a test
1. Add a test file in fsw/tests/
2. In fsw/ run 'make fsw_tests'
3. Execute ./build/fsw_test
