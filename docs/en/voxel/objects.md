# Objects
The **object** data type is for storing pairs of values. Object entries each consist of a key and their value, and both can be of any type. When something is used as a key, it will be locked and become immutable.

Classes and class intances are also objects.

Objects can be created in Voxel using the following syntax:

```voxel
var item = "banana";

var object = {
    "stringKey": 123,
    propertyKey: 456,
    789: true,
    data: null,
    message: "hello",
    fruit: item,
    list: [3, 2, 1],
    nested: {object: true}
};
```

## Reference

### `Object`
The object data type. All default properties can be overridden by getters defined on the object.

#### `length: Number`
The number of key-value pair entries in the object. This does not factor entries in prototypes, so it has an $ O(1) $ time complexity.

To find the length including entries from prototypes, use `Object.keys.length`.

#### `keys: [*]`
An immutable list of unique keys in this object and its prototypes.

#### `values: [*]`
A list of values in this object and its prototypes.

#### `pairs: {key: *, value: *}`
A list of objects containing keys (each `key`) and values (each `value`) in this object and its prototypes.

#### `prototypes: [Object]`
A mutable list of prototypes that the object inherits. When a key is accessed on the object and the object does not have an entry with the same key, each inherited object in the object's prototypes list will be looked up to find the key, starting from the last prototype in the list.