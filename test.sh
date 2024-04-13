#!/bin/bash

./build.sh --runtime

deno run --allow-read --allow-write --allow-run test/test.js