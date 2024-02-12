#!/bin/bash

mkdir -p dist

echo -n > dist/libvoxel.c

function include {
    cat $1 >> dist/libvoxel.c
    echo >> dist/libvoxel.c
}

include src/config.c
include src/voxel.c

cp src/exports.h dist/libvoxel.h

gcc -Wall -Wextra -g -c dist/libvoxel.c -o dist/libvoxel.o

if [ "$1" == "--examples" ]; then
    mkdir -p examples/build

    gcc -Idist/ examples/hello.c -g dist/libvoxel.o -o examples/build/hello
fi