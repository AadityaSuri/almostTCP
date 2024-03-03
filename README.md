## Documentation

See Doxygen documentation in doxygen/html/index.html

## Testing

To run the tests, run the following command from the project directory:

chmod +x tests/run_all_tests (only needed for the first time)
./tests/run_all_tests

The tests will run the receiver and sender programs with various bandwidth limits and packet drop rates. The tests check if the receiver receives the file correctly and if the output file is identical to the input file.

