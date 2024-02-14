#!/bin/bash

mkdir -p dist

echo -n > dist/libvoxel.h

function include {
    (
        echo "// $1"
        echo
        cat $1
        echo
        echo
    ) >> dist/libvoxel.h
}

cp src/config.h dist/libvoxel-config.h

tee -a dist/libvoxel.h > /dev/null << EOF
#ifndef LIBVOXEL_H_
#define LIBVOXEL_H_

EOF

include src/common.h
include src/errors.h
include src/context.h
include src/things.h
include src/numbers.h
include src/parser.h
include src/voxel.h

tee -a dist/libvoxel.h > /dev/null << EOF
#endif
EOF

if [ "$1" == "--examples" ]; then
    mkdir -p examples/build

    gcc -Idist/ examples/hello.c -o examples/build/hello
fi