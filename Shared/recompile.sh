#!/bin/bash 

rm -r build;
cmake -S . -B build;

cd build;
make;
