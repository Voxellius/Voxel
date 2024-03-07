# Voxel
Runtime and utilities for the Voxel programming language and VxC virtual machine.

## Building a Voxel project into VxC bytecode
You must first have Deno installed.

To compile Voxel source code (.vxl files) into VxC bytecode (.vxc files), run:

```bash
deno run --allow-read --allow-write tools/vxbuild-js/vxbuild.js examples/hello.vxl -o examples/build/hello.vxc
```

## Building
To build the Voxel runtime, run:

```bash
./build.sh --runtime
```

You can then run a .vxc file like this:

```bash
runtime/build/voxel code.vxc
```

To build the libvoxel integration example, then run the `hello` example code, run:

```bash
./build.sh --examples && examples/build/hello
```