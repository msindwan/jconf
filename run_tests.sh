#! /bin/bash

echo ''
echo 'Rebuilding test suite...'
echo ''

make clean && make

echo ''
echo 'Running tests...'
echo ''

./jconftest