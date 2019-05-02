# Builds and runs all fsw tests
echo "\nCompiling fsw_test...\n"
make fsw_test
echo "\n---- Running Tests ----\n"
sudo ./build/fsw_test -v
echo "-----------------------\n"
