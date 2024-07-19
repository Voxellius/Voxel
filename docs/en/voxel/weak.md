# Weak references
A **weak reference** is a type of thing that references a value while still permitting the value to be garbage-collected if there are no strong references to it.

This is required for circular references under reference-counting systems — like Voxel's — so that all referenced things can be garbage-collected after use.

A weak reference can be created for any thing by calling the `weak` method on it, which will return the weak reference. The referenced value can then be obtained by calling the `deref` method on the reference. If the referenced value has since been garbage-collected, `deref` will return `null` instead.

## Reference

### `WeakReference`
The weak reference data type.

#### `deref(): *`
Return the referenced thing. If the thing has since been garbage-collected, return `null` instead.