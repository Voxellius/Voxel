import * as codeGen from "./codegen.js";

export class Namespace {
    constructor(name = null) {
        this.name = name;
        this.symbols = {};
    }
}

export class Symbol {
    constructor(namespace, name) {
        this.namespace = namespace;
        this.name = name;
        this.code = codeGen.string(this.id);

        namespace.symbols[name] ??= [];

        namespace.symbols[name].push(this);
    }

    get id() {
        if (this.namespace.name == null) {
            return this.name;
        }

        return `${this.namespace.name}:${this.name}`;
    }

    generateCode() {
        return this.code;
        // return codeGen.string(`${this.namespace.name}_${this.name}`);
    }
}

export class SystemCall extends Symbol {
    generateCode() {
        return codeGen.systemCall(this.name);
    }
}

export function mangleSymbols(namespaces) {
    var symbolCollections = namespaces.map((namespace) => Object.values(namespace.symbols)).flat();

    var i = 0;

    // Sort symbol collections by frequency so more commonly-used symbols are given shorter VxC code representations
    symbolCollections = symbolCollections.sort((a, b) => b.length - a.length);

    for (var symbolCollection of symbolCollections) {
        for (var symbol of symbolCollection) {
            symbol.code = codeGen.number(i);
        }

        i++;
    }
}