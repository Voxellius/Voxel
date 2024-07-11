# Spread and rest syntax (`...`)
`...` can mean different things depending on where it is used in code. If it prepends an identifier in a parameter list, the associated parameter is known as a **rest parameter**. If it is used to prepend a value in an argument list, it acts as a **spread operator**. These are powerful features that can be used in Voxel to allow for variadic functions and to simplify library interfaces.

## Rest syntax
Functions in Voxel can take any number of arguments. This means that you can define a function that can take in an arbitrary number of arguments and store the list of arguments as a parameter:

```voxel
function sum(...values) {
    return values.reduce(function(accumulator, value) {
        return accumulator + value;
    }, 0);
}
```

In this example, `value` will be a list of the arguments passed to the `sum` function. The `sum` function will then add the values in the list together:

```voxel
sum(2, 3, 4); // -> 9
```

While you can only have one rest parameter, Voxel allows other parameters before and after the rest parameter:

```voxel
import io;

function listArgs(first, second, ...rest, penultimate, last) {
    io.println("First:", first);
    io.println("Second:", second);
    io.println("Rest:", rest);
    io.println("Penultimate:", penultimate);
    io.println("Last:", last);
}
```

When there are less arguments passed than parameters expected, all other non-rest parameters will be `null`. Voxel will first fill parameters from first parameter up to (and excluding) the rest parameter, then from the last parameter up to (and excluding) the rest parameter (so last gets filled first), then finally the rest parameter. This means that when calling `listArgs(1, 2, 3)`, `first` will be `1`, `second` will be `2`, `last` will be `3`, `rest` will be `[]` and `penultimate` will be `null`.

## Spread syntax
Since — as we have learned in the previous section — Voxel functions can take in any number of arguments, it is also possible to populate arguments from a list.

Going back to the `sum` function in the previous example, we can define a list and then use the values from that list as arguments to the function:

```voxel
var values = [5, 6, 7];

sum(...values); // -> 18
```

The spread operator can be applied multiple times in the same argument list, and alongside standard arguments:

```voxel
sum(1, ...[2, 3, 4], 5, ...[6, 7, 8]); // -> 36
```

The function being called using spread syntax does not need a rest parameter — all parameters are simply populated:

```voxel
function printArgs(a, b, c) {
    io.println("A:", a);
    io.println("B:", b);
    io.println("C:", c);
}

printArgs(...[1, 2, 3, 4, 5]);
```

This will produce the following output:

```
A: 1
B: 2
C: 3
```

The values `4` and `5` will be ignored in this example as they have no associated parameter for their position, and there is no rest parameter present.