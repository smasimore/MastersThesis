# FlightSoftware
All code flying on the rocket must be in this repository.

## Running Tests
sh ./RunTests.sh

## Cpputest
CppUTest is used to create unit & integration tests for flight software. 
[Cpputest Manual](https://cpputest.github.io/manual.html)

### Building cpputest library (only needs to be done once per machine)
1. cd libs/cpputest/cpputest_build
2. autoreconf .. -i
3. ../configure

### Creating a test
1. Add a test file in fsw/tests/
2. Run sh ./RunTests.sh to verify your test is now running as part of the FSW test suite
