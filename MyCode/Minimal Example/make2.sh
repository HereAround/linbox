#!/bin/bash

echo "This is a trick to compile MinimalExample.C - I have little idea as to how and why the following commands work, but they seem to."
echo "Compiling..."

g++ -DHAVE_CONFIG_H -I. -I..  -I.. -O2 -Wall -g -DNDEBUG -U_LB_DEBUG -DDISABLE_COMMENTATOR -fopenmp -fabi-version=6 -I/home/i/HDD/Computer/Mathematics_software/fflas-ffpack/build/include -I/home/i/HDD/Computer/Mathematics_software/givaro/build/include -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -mfma -MT MinimalExample.o -MD -MP -MF $depbase.Tpo -c -o MinimalExample.o MinimalExample.C

echo ""
echo "Created nullspace.o"
echo "Linking..."
echo ""

/bin/bash ../libtool --tag=CXX --mode=link g++ -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -mfma -o MinimalExample MinimalExample.o -L/home/i/HDD/Computer/Mathematics_software/givaro/build/lib -fopenmp -lblas -lgivaro -lgmp -lgmpxx ../linbox/liblinbox.la

echo ""
echo "Done"
