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
        this.foreignSymbolsToResolve = [];
        this.scope = null;

        existingNamespaces[this.sourceContainer.location] = this;
    }

    import(location, identifier) {
        this.importsToResolve[identifier] = path.resolve(path.dirname(this.sourceContainer.location), location);
    }

    hasImport(namespaceIdentifier) {
        return this.importsToResolve[namespaceIdentifier] || this.imports[namespaceIdentifier];
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

        for (var symbolReference of this.foreignSymbolsToResolve) {
            symbolReference.resolveSymbol();
        }
    }

    generateAst() {
        var tokens = tokeniser.tokenise(this.sourceContainer);

        return parser.parse(tokens, this);
    }

    getImportedNamespaces() {
        return [...new Set(Object.values(this.imports))];
    }

    async build(options) {
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

        if (options.mangle) {
            mangleSymbols(processedNamespaces);
        }

        for (var i = 0; i < processedNamespaces.length; i++) {
            var namespace = processedNamespaces[i];
            var ast = asts[i];

            if (namespace == coreNamespace) {
                console.log("Performing static code analysis for core namespace...");
            } else {
                console.log(`Performing static code analysis for \`${namespace.sourceContainer.name}\`...`);
            }

            ast.checkSymbolUsage();

            namespace.scope = ast.scope;
        }

        console.log("Resolving foreign symbol usage...");

        for (var i = 0; i < processedNamespaces.length; i++) {
            var namespace = processedNamespaces[i];

            function resolveForeignSymbolUsesForScope(scope) {
                for (var foreignSymbolUsage of scope.foreignSymbolUses) {
                    var subjectNamespace = (
                        foreignSymbolUsage.foreignNamespaceIdentifier ?
                        namespace.imports[foreignSymbolUsage.foreignNamespaceIdentifier] :
                        coreNamespace
                    );
    
                    var usage = subjectNamespace.scope.getSymbolById(Symbol.generateId(subjectNamespace, foreignSymbolUsage.name));
    
                    usage.readBy = foreignSymbolUsage.readBy;
                }

                for (var childScope of scope.childScopes) {
                    resolveForeignSymbolUsesForScope(childScope);
                }
            }

            resolveForeignSymbolUsesForScope(namespace.scope);
        }

        for (var i = 0; i < 16; i++) {
            console.log(`Pruning symbol usage (pass ${i + 1})...`);

            var anyPruned = false;

            for (var ast of asts) {
                anyPruned ||= ast.pruneSymbolUsage();
            }

            if (!anyPruned) {
                break;
            }
        }

        for (var i = 0; i < processedNamespaces.length; i++) {
            var namespace = processedNamespaces[i];
            var ast = asts[i];

            if (options.analyseAst) {
                console.log("Analysed AST:", ast.analyse());
            }

            if (namespace == coreNamespace) {
                console.log("Generating VxC code for core namespace...");
            } else {
                console.log(`Generating VxC code for \`${namespace.sourceContainer.name}\`...`);
            }

            code.push(ast.generateCode(options));
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

    static generateId(namespace, name) {
        if (namespace == coreNamespace) {
            return `#core:${name}`;
        }

        return `${namespace.id}:${name}`;
    }

    get id() {
        return this.constructor.generateId(this.namespace, this.name);
    }

    generateCode(options) {
        return this.code;
    }
}

export class SystemCall extends Symbol {
    generateCode(options) {
        return codeGen.systemCall(this.name);
    }
}

export class ForeignSymbolReference {
    constructor(receiverNamespace, subjectNamespaceIdentifier, symbolName) {
        this.receiverNamespace = receiverNamespace;
        this.subjectNamespaceIdentifier = subjectNamespaceIdentifier;
        this.symbolName = symbolName;

        this.symbol = null;

        this.receiverNamespace.foreignSymbolsToResolve.push(this);
    }

    resolveSymbol() {
        if (this.symbol != null) {
            return;
        }

        this.symbol = new Symbol(this.receiverNamespace.imports[this.subjectNamespaceIdentifier], this.symbolName);
    }

    generateCode(options) {
        return this.symbol.generateCode(options);
    }
}

export class SymbolUsage {
    constructor(id) {
        this.id = id;

        this.everDefined = false;
        this.readBy = [];
        this.truthiness = null;
    }

    get everRead() {
        return this.readBy.length > 0;
    }

    updateTruthiness(truthiness) {
        if (!this.everDefined && !this.everRead) {
            this.everDefined = true;
            this.truthiness = truthiness;
        }

        if (this.truthiness == null) {
            return;
        }

        if (this.truthiness != truthiness) {
            this.truthiness = null;
        }
    }
}

export class ForeignSymbolUsage {
    constructor(name, foreignNamespaceIdentifier = null) {
        this.name = name;
        this.foreignNamespaceIdentifier = foreignNamespaceIdentifier;

        this.readBy = [];
    }
}

export class Scope {
    constructor() {
        this.parentScope = null;
        this.childScopes = [];
        this.symbolUses = [];
        this.foreignSymbolUses = [];
    }

    getSymbolById(id, defining = false) {
        var usage = this.symbolUses.find((usage) => usage.id == id);

        if (!usage && !defining && this.parentScope != null) {
            usage = this.parentScope.getSymbolById(id);
        }

        if (usage) {
            return usage;
        }

        if (!defining) {
            console.warn(`Undefined symbol: ${id}`);
        }

        usage = new SymbolUsage(id);

        this.symbolUses.push(usage);

        return usage;
    }

    addSymbol(symbol, reading = true, defining = false, reader = null) {
        if (!(symbol instanceof Symbol)) {
            return null;
        }

        var usage = this.getSymbolById(symbol.id, defining);

        if (reading) {
            usage.readBy.push(reader);
        }

        usage.everDefined ||= defining;

        return usage;
    }

    addCoreNamespaceSymbol(symbol) {
        if (!(symbol instanceof Symbol)) {
            return null;
        }

        if (this.foreignSymbolUses.find((usage) => usage.name == symbol.name && usage.foreignNamespaceIdentifier == null)) {
            return true;
        }

        var usage = new ForeignSymbolUsage(symbol.name);

        this.foreignSymbolUses.push(usage);

        return usage;
    }

    getSymbolTruthiness(symbol) {
        if (symbol instanceof Symbol) {
            return this.symbolUses.find((usage) => usage.id == symbol.id)?.truthiness ?? null;
        }

        if (symbol instanceof ForeignSymbolReference) {
            return this.foreignSymbolUses.find((usage) => (
                usage.name == symbol.name &&
                usage.foreignNamespaceIdentifier == symbol.subjectNamespaceIdentifier
            ))?.truthiness ?? null;
        }

        return null;
    }

    createChildScope() {
        var instance = new this.constructor();

        instance.parentScope = this;

        this.childScopes.push(instance);

        return instance;
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