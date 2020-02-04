#!/bin/bash

echo "This is a trick to compile nullspace.C - I have little idea as to how and why the following commands work, but they seem to."
echo "Create nullspace.o"

g++ -DHAVE_CONFIG_H -I. -I..  -I.. -O2 -Wall -pg -U_LB_DEBUG -DDISABLE_COMMENTATOR -fopenmp -fabi-version=6 -I/home/i/HDD/Computer/Mathematics_software/fflas-ffpack/build/include -I/home/i/HDD/Computer/Mathematics_software/givaro/build/include -march=native -mfma -MT nullspacebasis.o -MD -MP -MF $depbase.Tpo -c -o nullspacebasis.o nullspacebasis.C

echo ""
echo "Create executable"
echo ""

g++ -static -march=native -mfma -o nullspacebasis nullspacebasis.o ../linbox/.libs/liblinbox.a -L/home/i/HDD/Computer/Mathematics_software/givaro/build/lib -fopenmp -lblas -lgivaro -lgmp -lgmpxx

echo ""
echo "Done"
