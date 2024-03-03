import * as codeGen from "./codegen.js";

export class Namespace {
    constructor(name = null) {
        this.name = name || "@";
        this.symbols = [];
    }
}

export class Symbol {
    constructor(namespace, name) {
        this.namespace = namespace;
        this.name = name;

        namespace.symbols.push(this);
    }

    generateCode() {
        return codeGen.string(this.name);
        // return codeGen.string(`${this.namespace.name}_${this.name}`);
    }
}