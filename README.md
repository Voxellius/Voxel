# Voxel
> Please refrain from sharing Voxel on social media platforms such as Hacker News and Reddit for the time being. Voxellius wishes to maintain control over the timing and extent that Voxel and its other projects are officially announced.

**Voxel** is a loosely-typed programming language that is designed to be powerful, versatile, easy-to-use and portable. It has a C-like syntax and has many features that can be found in other modern programming languages.

Portability is a key goal of Voxel — especially so that programs written in it can run on low-powered hardware, such as microcontrollers. Voxel achieves this by compiling source code into a bytecode format, called **VxC** (Voxel Code). Voxel can generate space-efficient bytecode by using a number of optimisations, including name mangling and dead code elimination (DCE) via static code analysis.

Voxel only generates the necessary bytecode to provide the supporting features needed by the main program's code. As an example, here's a hex dump of all the bytecode needed to output "Hello, world!":

```
56 78 43 01 22 48 65 6c 6c 6f 2c 20 77 6f 72 6c     VxC."Hello, worl
64 21 0a 00 33 01 2e 6c 6f 67 00 70 00              d!..3..log.p.
```

Since this program does not make use of Voxel's auxiliary features, all code from [`core.vxl`](tools/vxbuild-js/core.vxl) in this example has been excluded through static code analysis and is therefore never compiled into the bytecode.

The following Voxel code produced the above bytecode:

```voxel
syscall log("Hello, world!\n");
```

More examples of Voxel programs can be found in the [`examples`](examples) directory.

## Key features of the Voxel programming language
* **Functions** — named and anonymous
    * **Closures** that allow references to non-global variables outside a function's scope
* **Expressions** that follow a defined operator precedence
    * **Short-circuit evaluation** of logical operators using `&&` and `||` (evaluation of subsequent operands is aborted if result's value is guaranteed to be `true` or `false`)
    * **Eager evaluation** of logical operators using `&&&` and `|||` (all operands are evaluated — this produces simpler bytecode but may perform unnecessary computations at runtime)
* **Lists** and their common operations (such as access through index accessors and `push` and `pop` methods)
* **Objects**
    * **Object-oriented programming** features including classes and prototype-based multiple inheritance
    * **Methods** that have access to their associated object by using `this` keyword, including constructor methods that are called by using `new`
    * **Getters and setters** to simplify object interfaces and perform data validation (by prefixing class method declarations with `get` or `set`)
    * **Calling parent methods** by using the `super` keyword to call the constructor method (`super()`) or other methods (`super.methodName()`)
* **Imports** of other Voxel files and libraries
    * **Modular, namespaced approach** where each imported file is called in the correct dependency order and where symbol names across files won't collide
    * **Relative imports** by specifying a path relative to the Voxel file
    * **Circular imports** where _A_ can import _B_ and _B_ can import _A_
* **Exceptions** to throw and handle errors with `throw` and `try`/`catch`
* **Static macros** for libraries to determine whether symbols and properties are used or not to enable efficient code generation
* **Dead code elimination** to produce space-efficient VxC bytecode
    * **Truthiness estimation** so that unreachable code in `if` statements and `while` loops (due to condition always being `true` or `false`) is removed
    * **Tree shaking** to remove functions that are never called and variables that are never read
    * **Potential side effect detection** when deciding how unused variables should be removed so that their assigned value is still evaluated if its expression contains a function call or getter method
* **Multithreading** through the `threads` library, which allows multiple functions to be executed concurrently

## Key features of the VxC bytecode format
* **Stack-based virtual machine** that enables powerful manipulation of data and easy passing of data as arguments to commands
* **Position-independent code** to allow easy linkage of libraries without absolute address references
* **Read-only execution** to reduce potential for exploits whereby untrusted code is loaded and executed
    * Sandboxed, untrusted code could be allowed to execute by creating a syscall in C that instantiates a new Voxel context with `voxel_newContext` and loads in the byte buffer from a syscall argument as code for execution
* **Compact code** with a space-efficient design
* **Easy-to-read instruction set** with all instruction tokens as printable ASCII characters
* **Loose typing of data** with type casting where necessary

## Compiling a Voxel project into VxC bytecode
Before compiling projects, you must first have Deno installed.

To compile Voxel source code (.vxl files) into VxC bytecode (.vxc files), run:

```bash
deno run --allow-read --allow-write tools/vxbuild-js/vxbuild.js examples/hello.vxl -o examples/build/hello.vxc
```

## Building and running the Voxel runtime
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

## Testing all features
To test all features of Voxel, run:

```bash
./test.sh
```

This will perform all tests listed in the `test` directory against their expected outputs and for memory leaks (the latter runs a test's program in an infinite loop and measures the difference in memory consumption).