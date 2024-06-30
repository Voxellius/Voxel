# Buffers
The **buffer** data type is for storing a fixed-size array of byte values.

Buffers can be created statically in Voxel using the buffer literal syntax:

```voxel
var data = Buffer(0x12, 0xF2, 0xBA, 0x40, "string");
```

The buffer literal syntax can contain number and string literals — numbers will be converted into bytes (only the lowest 8 bits will be used), and strings will be encoded as UTF-8 bytes and will not be null-terminated.

To create buffers with a computed size at runtime, the `create(size: Number, value?: Byte)` function of the `buffers` standard library can be used instead.

## Reference

### `Buffer`
The buffer data type.

#### `size: Number`
The number of bytes in the buffer.

#### `fill(value: Byte, start?: Number, end?: Number)`
Fill the buffer with the same given byte value (`value`) from the start index (`start`) up to the end index (`end`). The end index will not be filled.

If the start index is not specified, then it will default to `0`. If the end index is not specified, then it will default to the size of the buffer.

#### `copy(destination: Buffer, offset?: Number, start?: Number, end?: Number)`
Copy data from the buffer (the source buffer) to the destination buffer (`destination`). The source buffer will be read from the start index (`start`) up to the end index (`end`). The end index will not be included. The destination buffer will be written to from the offset index (`offset`).

If the start index is not specified, then it will default to `0`. If the offset index is not specified, then it will also default to `0`. If the end index is not specified, then it will default to the size of the source buffer.

#### `section(start: Number, end?: Number)`
Return a new buffer that is a section of the buffer from the start index (`start`) up to the end index (`end`). The end index will not be included.

If the end index is not specified, then it will default to the size of the buffer.