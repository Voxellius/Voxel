# Weak references
A **weak reference** is a type of thing that references a value while still permitting the value to be garbage-collected if there are no strong references to it.

This is required for circular references under reference-counting systems — like Voxel's — so that all referenced things can be garbage-collected after use.

A weak reference can be created for any thing by calling the `weak` method on it, which will return the weak reference. The referenced value can then be obtained by calling the `deref` method on the reference. If the referenced value has since been garbage-collected, `deref` will return `null` instead.

## Example
This example constructs a hierarchical tree with a two-way relationship between parent and child nodes. Here, nodes have a weak reference to their parent and strong references to their children. The example then calls the `getPath` method on one of the nodes, which traverses the node's parents recursively to construct a path string.

```vxl
import io;

class TreeNode {
    parent = null;
    children = [];

    constructor(name) {
        this.name = name;
    }

    addChild(child) {
        child.parent = this.weak();

        this.children.push(child);
    }

    getPath() {
        if (this.parent == null) {
            return this.name;
        }

        return this.parent.deref().getPath() + " > " + this.name;
    }
}

var root = new TreeNode("Root");
var node1 = new TreeNode("Node 1");
var node2 = new TreeNode("Node 2");
var subnode1 = new TreeNode("Subnode 1");
var subnode2 = new TreeNode("Subnode 2");

root.addChild(node1);
root.addChild(node2);

node1.addChild(subnode1);
node1.addChild(subnode2);

io.println(subnode2.getPath());
```

## Reference

### `WeakReference`
The weak reference data type.

#### `deref(): *`
Return the referenced thing. If the thing has since been garbage-collected, return `null` instead.