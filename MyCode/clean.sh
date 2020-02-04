#!/bin/bash

if [ -e nullspacebasis ]
then
    rm nullspacebasis
fi

if [ -e nullspacebasis.o ]
then
    rm nullspacebasis.o
fi

if [ -e gmon.out ]
then
    rm gmon.out
fi

if [ -e .Tpo ]
then
    rm .Tpo
fi
