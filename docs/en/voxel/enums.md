# Enumerations (enums)
Enumerations are a compile-time language feature of Voxel that allow a set of discrete named values to be created. These values are statically converted into numbers at compile-time and inlined into the generated bytecode. Enumerations are defined by providing a name for the full set of entries and a name for each entry in the enumeration (and optionally an assigned numerical value).

Here is an example of an enumeration to describe a number of modes of transport:

```voxel
enum ModeOfTransport {
    WALKING = 0,
    BICYCLE,
    CAR,
    BUS,
    TRAIN,
    BOAT,
    AEROPLANE
}
```

Here, the entry `ModeOfTransport.WALKING` is explicitly assigned the value `0`. All other values will automatically be assigned an implicit, unique value when compiled.

> [!NOTE]
> Explicitly assigned values must be number literals or string literals (for the latter, the code of the first character of the string literal will be the value).

Enumerated values can then be stored in variables:

```voxel
import io;

var outboundMode = ModeOfTransport.BUS;
var returnMode = ModeOfTransport.TRAIN;

if (outboundMode == ModeOfTransport.BUS) {
    io.println("Please ensure that you check the bus timetable before you leave as the buses are no longer every 15 minutes.");
}
```

Implicitly generated values for entries that have not been assigned an explicit value will be **globally unique** — that is, they will not share a generated implicit value that is equal to any other value in the same enumeration (including **explicit** values) or is equal to any **implicit** value (excluding **explicit** values) of other enums, both in the same module or other present modules. This is because enumeration entries are converted to numbers in bytecode, so this ensures that when comparing entries between different enumerations, the result is `false`. Explicitly assigned values are allowed to be non-unique.

## Getting enumerator identifiers
Because entry values are numbers, when entries are printed, they will only show the value. To obtain a string representation of an enumeration entry by its value, you can use the `Number.getEnumEntry()` method:

```voxel
var someValue = ModeOfTransport.CAR;

io.println(someValue.getEnumEntry()); // -> "main:ModeOfTransport.CAR"
```

This can be used for debugging purposes to determine the entry identifier for a value. If an enumeration entry cannot be found for a value, the value will be returned instead.

> [!NOTE]
> The returned value from `getEnumEntry` will be a string that contains the originating module's namespace identifier (usually derived from the module's filename). For this reason, it is recommended to avoid performing comparisons with the returned value.

### Enumeration lookup map
In order for the `getEnumEntry` method to look up the original entry's identifier, VxBuild will include all used enumeration entries' identifiers and values in the generated bytecode to register them for the Voxel runtime's enumeration lookup map. To prevent this, use the `--no-enum-lookup` flag on VxBuild.

All unused enumeration entries are not registered in the generated bytecode unless dead code elimination is disabled with VxBuild's `--no-dce` flag.

## Enumerations as error codes
Voxel's preferred means of error handling is by throwing enumeration entries. This is because less bytecode is needed when compared to throwing class instances (which is also possible but requires more storage space for enabling object-oriented programming). Any uncaught numerical values are then looked up in the enumeration lookup map to generate an appropriate error message.

The following code will produce the error as shown below it:

```voxel
enum HttpStatus {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500
}

var status = HttpStatus.NOT_FOUND;

if (status != HttpStatus.OK) {
    throw status;
}
```

```
Unhandled exception: encountered error code main:HttpStatus.NOT_FOUND (404)
```
