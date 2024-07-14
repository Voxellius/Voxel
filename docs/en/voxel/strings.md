# Strings
The **string** data type is for storing textual data. String literals start and end with either `"`, `'` or `` ` ``.

A number of escape sequences can be used to insert special characters into strings that are non-printable or would otherwise be syntactically invalid:

| Escape sequence | Meaning |
|-|-|
| `\\` | Backslash (`\`) |
| `\r` | Carriage return |
| `\n` | Newline |
| `\"` | Double quote (`"`) |
| `\'` | Single quote (`'`) |
| `` \` `` | Backtick (`` ` ``) |

## Reference

### `String`
The string data type.

#### `toNumber(): Number`
Convert the string to a [number](numbers.md).

#### `toBuffer(): Buffer`
Convert the string to a [buffer](buffers.md).

#### `size: Number`
The number of bytes in the string.

#### `reverse(): String`
Return a reversed version of the string.

#### `cutStart(maxSize: Number): String`
Return a cut version of the string where characters at the start are removed if the string is longer than the maximum size (`maxSize`).

#### `cutEnd(maxSize: Number): String`
Return a cut version of the string where that characters at the end are removed if the string is longer than the maximum size (`maxSize`).

#### `padStart(minSize: Number, fill?: String): String`
Return a padded version of the string where it is at least the minimum size (`minSize`), adding characters at the start using the fill string (`fill`). The fill string will be repeated where necessary.

If the fill string is not specified, then it will default to `" "`.

#### `padEnd(minSize: Number, fill?: String): String`
Return a padded version of the string where it is at least the minimum size (`minSize`), adding characters at the end using the fill string (`fill`). The fill string will be repeated where necessary.

If the fill string is not specified, then it will default to `" "`.

#### `substring(start: Number, end?: Number): String`
Return a new string that is a section of the string from the start index (`start`) up to the end index (`end`). The end index will not be included.

If the end index is not specified, then it will default to the size of the string.