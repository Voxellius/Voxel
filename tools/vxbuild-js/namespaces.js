import * as path from "https://deno.land/std@0.220.1/path/mod.ts";

import * as sources from "./sources.js";
import * as tokeniser from "./tokeniser.js";
import * as parser from "./parser.js";
import * as codeGen from "./codegen.js";

var generatedSymbolIndex = 0;
var namespaceIndex = 0;
var existingNamespaces = {};

export var coreNamespace = null;

export class Namespace {
    constructor(sourceContainer = null) {
        this.sourceContainer = sourceContainer;

        this.id = String(namespaceIndex++);
        this.symbols = {};
        this.importsToResolve = {};
        this.imports = {};

        existingNamespaces[this.sourceContainer.location] = this;
    }

    import(location, identifier) {
        this.importsToResolve[identifier] = path.resolve(path.dirname(this.sourceContainer.location), location);
    }

    async resolveImports() {
        for (var identifier in this.importsToResolve) {
            var location = this.importsToResolve[identifier];

            if (existingNamespaces[location]) {
                this.imports[identifier] = existingNamespaces[location];

                continue;
            }

            var source = await Deno.readTextFile(location);
            var sourceContainer = new sources.SourceContainer(source, location);
            var namespace = new Namespace(sourceContainer);

            this.imports[identifier] = namespace;
        }

        this.importsToResolve = {};
    }

    generateAst() {
        var tokens = tokeniser.tokenise(this.sourceContainer);

        return parser.parse(tokens, this);
    }

    getImportedNamespaces() {
        return [...new Set(Object.values(this.imports))];
    }

    async build(mangle = false) {
        var processedNamespaces = [];
        var allDiscoveredNamespaces = [];
        var asts = [];
        var code = [];

        async function processNamespace(namespace) {
            if (allDiscoveredNamespaces.includes(namespace)) {
                return;
            }

            if (namespace == coreNamespace) {
                console.log("Parsing core namespace...");
            } else {
                console.log(`Parsing \`${namespace.sourceContainer.name}\`...`);
            }

            var ast = namespace.generateAst();

            allDiscoveredNamespaces.push(namespace);

            await namespace.resolveImports();

            for (var importedNamespace of namespace.getImportedNamespaces()) {
                await processNamespace(importedNamespace);

                allDiscoveredNamespaces.push(importedNamespace);
            }

            processedNamespaces.push(namespace);
            asts.push(ast);
        }

        await processNamespace(coreNamespace);
        await processNamespace(this);

        if (mangle) {
            mangleSymbols(processedNamespaces);
        }

        for (var i = 0; i < processedNamespaces.length; i++) {
            var namespace = processedNamespaces[i];
            var ast = asts[i];

            if (namespace == coreNamespace) {
                console.log("Generating VxC code for core namespace...");
            } else {
                console.log(`Generating VxC code for \`${namespace.sourceContainer.name}\`...`);
            }

            code.push(ast.generateCode());
        }

        return codeGen.join(...code);
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
        if (this.namespace == coreNamespace) {
            return `#core:${this.name}`;
        }

        return `${this.namespace.id}:${this.name}`;
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

export function generateSymbolName(prefix) {
    return `#${prefix}_${generatedSymbolIndex++}`;
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

export async function init() {
    var location = path.resolve(path.dirname(path.fromFileUrl(Deno.mainModule)), "core.vxl");
    var source = await Deno.readTextFile(path.resolve(location));
    var sourceContainer = new sources.SourceContainer(source, path.resolve(location));

    coreNamespace = new Namespace(sourceContainer);
}