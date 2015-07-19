#! /bin/bash

echo ''
echo 'REBUILDING TEST SUITE...'

make clean > /dev/null && make test > /dev/null

echo ''

./bin/jconftest
make clean > /dev/null