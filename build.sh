#!/bin/sh
mkdir -p build
cd build
cmake ..
cmake --build .
cd ..
mkdir -p dist
mv build/Debug/* dist
cp paddle_1.mp3 paddle_2.mp3 pong_start-score.mp3 dist
