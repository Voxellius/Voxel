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

#### `length: Number`
The number of characters (Unicode code points) in the string.

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

#### `byteAt(index: Number)?: Byte`
Get the byte at a specified index (`index`) in the string, or `null` if the index is out-of-bounds.

The index refers to the byte position in the string instead of the character position.

#### `substring(start: Number, end?: Number): String`
Return a new string that is a section of the string from the start index (`start`) up to the end index (`end`). The end index will not be included.

If the end index is not specified, then it will default to the size of the string.

#### `repeat(times: Number): String`
Return a new string that is a repetition of the string for a given number of times (`times`). If the number of times is _n_, then the returned string will be _n_ times the size of the original string.

#### `startsWith(target: String): Boolean`
Return `true` if the string starts with the target string (`target`).

#### `endsWith(target: String): Boolean`
Return `true` if the string ends with the target string (`target`).

#### `split(delimeter: String): List<String>`
Split the string into a list and return the list, separating the string by the given delimeter (`delimeter`).