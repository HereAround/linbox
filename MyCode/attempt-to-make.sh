#!/bin/bash

echo "This is a trick to compile nullspace.C - I have little idea as to how and why the following commands work, but they seem to."
echo "Compiling..."

g++ -DHAVE_CONFIG_H -I. -I..  -I.. -O2 -Wall -pg -DNDEBUG -U_LB_DEBUG -DDISABLE_COMMENTATOR -fopenmp -fabi-version=6 -I/home/i/HDD/Computer/Mathematics_software/fflas-ffpack/build/include -I/home/i/HDD/Computer/Mathematics_software/givaro/build/include -march=native -mfma -MT nullspacebasis.o -MD -MP -MF $depbase.Tpo -c -o nullspacebasis.o nullspacebasis.C

echo ""
echo "Created nullspace.o"
echo "Linking..."
echo ""

/bin/bash ../libtool --tag=CXX --mode=link g++ -pg -march=native -mfma -o nullspacebasis nullspacebasis.o -L/home/i/HDD/Computer/Mathematics_software/givaro/build/lib -fopenmp -lblas -lgivaro -lgmp -lgmpxx ../linbox/liblinbox.la

echo ""
echo "Done"
