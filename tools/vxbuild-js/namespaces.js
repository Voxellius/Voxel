export class Namespace {
    constructor() {
        this.symbols = [];
    }
}

export class Symbol {
    constructor(namespace, name) {
        this.namespace = namespace;
        this.name = name;

        namespace.symbols.push(this);
    }
}