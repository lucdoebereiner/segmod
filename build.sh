#!/usr/bin/env bash

g++ -std=c++0x segmod2.cpp -lsndfile -o segmod2 
emcc --bind segmod2.cpp -s ALLOW_MEMORY_GROWTH=1 -o web/segmod2.out.js

