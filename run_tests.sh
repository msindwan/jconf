#! /bin/bash

echo ''
echo 'REBUILDING TEST SUITE...'

make clean > /dev/null && make test > /dev/null

echo ''

./jconftest
make clean > /dev/null