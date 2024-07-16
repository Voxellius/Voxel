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
include src/declarations.h
include src/builtins/core/maths.h
include src/builtins/core/buffers.h
include src/builtins/core/strings.h
include src/builtins/core/objects.h
include src/builtins/core/lists.h
include src/builtins/core/weak.h
include src/builtins/core/core.h
include src/builtins/io/io.h
include src/builtins/threads/threads.h
include src/maths.h
include src/contexts.h
include src/things.h
include src/null.h
include src/booleans.h
include src/bytes.h
include src/functions.h
include src/closures.h
include src/numbers.h
include src/buffers.h
include src/strings.h
include src/objects.h
include src/lists.h
include src/weak.h
include src/enums.h
include src/operations.h
include src/parser.h
include src/executors.h
include src/scopes.h
include src/helpers.h
include src/voxel.h

tee -a dist/libvoxel.h > /dev/null << EOF
#endif
EOF

function doJob() {
    echo "$1..."
    eval "$2"
    echo "Done $1."
}

if [ "$1" == "--examples" ]; then
    mkdir -p examples/build

    doJob "Compiling examples" "gcc -Idist/ examples/hello.c -o examples/build/hello"
fi

if [ "$1" == "--runtime" ]; then
    mkdir -p runtime/build

    doJob "Compiling runtime" "gcc -Idist/ runtime/voxel.c -o runtime/build/voxel"

    if [ "$2" == "--zip" ]; then
        if [ "$3" == "-e" ] && [ -f runtime/build.zip ]; then
            rm runtime/build.zip
            echo "Removed old build.zip"
        fi
        doJob "Creating zip of build" "zip -r runtime/build.zip runtime/build stdlib docs tools"
    fi
fi
