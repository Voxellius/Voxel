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

#### `keys: List<*>`
An immutable list of unique keys in this object and its prototypes.

#### `values: List<*>`
A list of values in this object and its prototypes.

#### `pairs: {key: *, value: *}`
A list of objects containing keys (each `key`) and values (each `value`) in this object and its prototypes.

#### `prototypes: List<Object<*, *>>`
A mutable list of prototypes that the object inherits. When a key is accessed on the object and the object does not have an entry with the same key, each inherited object in the object's prototypes list will be looked up to find the key, starting from the last prototype in the list.

#### `keyOf(value: *): Number`
Find the key of the first occurrence of a value (`value`) in the object and return it.

If the value cannot be found, `null` will be returned instead.

#### `forEach(callback: function(value: *, key: *))`
Iterate through the object and call a callback function (`callback`) for every entry pair with arguments that reference the value (`value`) and the key (`key`).

#### `map(callback: function(item: *, key: *): *): Object<*, *>>`
Iterate through the object and call a callback function (`callback`) for every entry pair with arguments that reference the value (`value`) and the key (`key`).

The callback function must return a new value. A new object will be returned when calling `map` containing all keys and their new values returned from calling the callback function for every entry.

#### `filter(callback: function(item: *, key: *): Boolean): List<*>`
Iterate through the object and call a callback function (`callback`) for every entry pair with arguments that reference the value (`value`) and the key (`key`).

The callback function must return a boolean value. A new object will be returned when calling `filter` containing all entries from the original object where the callback function returned `true`.

#### `find(callback: function(value: *, key: *): Boolean): *`
Iterate through the object until the callback function (`callback`) called for every entry pair returns `true`. Return the value of the entry pair that caused the callback function to return `true`. The callback function is called with arguments that reference the value (`value`) and the key (`key`).

If the callback function never returns `true`, then `null` will be returned instead.

#### `findKey(callback: function(value: *, key: *): Boolean): *`
Iterate through the object until the callback function (`callback`) called for every entry pair returns `true`. Return the value of the entry pair that caused the callback function to return `true`. The callback function is called with arguments that reference the value (`value`) and the key (`key`).

If the callback function never returns `true`, then `null` will be returned instead.

#### `reduce(callback: function(accumulator: *, value: *, key: *): *, initialValue: *): *`

Iterate through the object and call a callback function (`callback`) for every entry pair. The callback will be called with arguments for the previous call's return value or the initial provided value (`initialValue`) if it is the first call (`accumulator`), the current entry pair's value (`value`) and the pair's key (`key`). The last call's return value will be used as the return value of `reduce`.