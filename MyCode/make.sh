#!/bin/bash

echo ""
echo "Create nullspacebasis.o..."
echo ""

g++ -DHAVE_CONFIG_H -I. -I..  -I.. -O2 -Wall -pg -DNDEBUG -U_LB_DEBUG -DDISABLE_COMMENTATOR -fopenmp -fabi-version=6 -I/home/i/HDD/Computer/Mathematics_software/fflas-ffpack/build/include -I/home/i/HDD/Computer/Mathematics_software/givaro/build/include -march=native -mfma -MT nullspacebasis.o -MD -MP -MF $depbase.Tpo -c -o nullspacebasis.o nullspacebasis.C

echo ""
echo "Created executable nullspacebasis..."
echo ""

g++ -pg -march=native -mfma -o nullspacebasis nullspacebasis.o -fopenmp  -L/home/i/HDD/Computer/Mathematics_software/givaro/build/lib -lblas /home/i/HDD/Computer/Mathematics_software/givaro/build/lib/libgivaro.so -lgmp -lgmpxx ../linbox/.libs/liblinbox.so -fopenmp -Wl,-rpath -Wl,/home/i/HDD/Computer/Mathematics_software/givaro/build/lib -Wl,-rpath -Wl,/home/i/HDD/Computer/Mathematics_software/linbox/build/lib

echo ""
echo "Done"
