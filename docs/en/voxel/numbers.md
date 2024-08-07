# Numbers
The **number** data type is for storing a numerical value, such as an integer or floating-point number. 

Voxel's syntax supports many formats that can be used for number literals:

| Format | Example |
|-|-|
| Integer (basic) | `12` |
| Floating-point (basic) | `17.65` |
| Floating-point (scientific) | `3.58E-8` |
| Integer (binary; base-2) | `0b10010101` |
| Integer (octal; base-8) | `0o755` |
| Integer (hexadecimal; base-16) | `0x12FA` |

## Reference

### `Number`
The number data type.

#### `toString(base?: Number): Number`
Convert the number to a [string](string.md). If a base radix (`base`) is supplied, then format the number as a signed integer in the given base. If supplied, then the base radix must be between 2 and 16.

#### `toByte(): Byte`
Conver the number to a byte. The number will be converted to an integer (if it is not already an integer) and only the 8 least significant bits will be used.

#### `isNan(): Boolean`
Determine whether the numeric value is not a number (`nan`) and return `true` if this is the case.

#### `getEnumProp(): String | Number`
Get a string representation of the enumeration entry's identifier referenced by the number's value. If no associated enumeration entry is found, then return the number instead.

For more information, see [Enumerations (enums) § Getting enumerator identifiers](enums.md#getting-enumerator-identifiers).