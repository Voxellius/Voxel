#!/bin/bash

./build.sh --runtime

deno run --allow-all test/test.js