import * as sources from "./sources.js";
import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";
import * as dce from "./dce.js";
import * as parser from "./parser.js";
import * as statements from "./statements.js";
import * as staticMacros from "./staticmacros.js";

export class ThisNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`this`";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "this")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.getThisSymbol = new namespaces.Symbol(namespaces.coreNamespace, "getThis");

        this.eat(tokens);

        return instance;
    }

    checkSymbolUsage(scope) {
        scope.addCoreNamespaceSymbol(this.getThisSymbol, this);

        super.checkSymbolUsage(scope);
    }

    generateCode(options) {
        return codeGen.join(
            codeGen.number(0),
            this.getThisSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
        );
    }
};

export class SuperNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`super`";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "super")
    ];

    callingMethod = false;

    static create(tokens, namespace) {
        var instance = new this();

        instance.callSuperMethod = new namespaces.Symbol(namespaces.coreNamespace, "callSuperMethod");
        instance.callSuperConstructors = new namespaces.Symbol(namespaces.coreNamespace, "callSuperConstructors");

        this.eat(tokens);

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.PropertyAccessorToken)])) {
            instance.callingMethod = true;

            instance.propertySymbol = namespaces.Symbol.generateForProperty(this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value);
        }

        instance.expectChildByMatching(tokens, [ConstructorArgumentsNode], namespace);

        return instance;
    }

    checkSymbolUsage(scope) {
        if (this.callingMethod) {
            scope.addCoreNamespaceSymbol(this.callSuperMethod, this);
            namespaces.propertySymbolUses.push(scope.addSymbol(this.propertySymbol, true, false, this));
        } else {
            scope.addCoreNamespaceSymbol(this.callSuperConstructors, this);
        }

        super.checkSymbolUsage(scope);
    }

    generateCode(options) {
        var classNode = this.findAncestorOfTypes([ClassNode]);
        
        if (!classNode) {
            // TODO: Specify token
            throw new sources.SourceError("`super` must be used inside a class declaration");
        }

        if (this.callingMethod) {
            return codeGen.join(
                classNode.identifierSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET),
                this.propertySymbol.generateCode(options),
                this.children[0].generateCode(options),
                codeGen.number(3),
                this.callSuperMethod.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
            );
        }

        return codeGen.join(
            classNode.identifierSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET),
            this.children[0].generateCode(options),
            codeGen.number(2),
            this.callSuperConstructors.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
        );
    }
};

export class ThingNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "thing expression";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.AtomToken),
        new ast.TokenQuery(tokeniser.TypeLiteralToken),
        new ast.TokenQuery(tokeniser.IdentifierToken),
        new ast.TokenQuery(tokeniser.StringToken),
        new ast.TokenQuery(tokeniser.NumberToken)
    ];

    value = null;
    containedValueTokens = [];
    containerType = null;
    memoisedContainedTypeValue = null;
    bufferLength = null;

    static create(tokens, namespace) {
        var instance = new this();

        var token = this.eat(tokens);

        if (token instanceof tokeniser.AtomToken) {
            instance.value = {
                "true": true,
                "false": false,
                "null": null
            }[token.value] ?? null;
        } else if (token instanceof tokeniser.TypeLiteralToken) {
            this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")]);

            if (token.value == "Byte") {
                instance.containedValueTokens.push(tokens[0]);

                instance.expectChildByMatching(tokens, [ThingNode], namespace);

                this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")]);

                instance.containerType = "byte";
            } else if (token.value == "Buffer") {
                var addedFirstItem = false;

                while (true) {
                    if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")])) {
                        break;
                    }

                    if (addedFirstItem) {
                        this.eat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)]);
                    }

                    instance.containedValueTokens.push(tokens[0]);

                    instance.expectChildByMatching(tokens, [ThingNode], namespace);

                    addedFirstItem = true;
                }

                instance.containerType = "buffer";
            } else {
                throw new Error("Not implemented");
            }
        } else if (token instanceof tokeniser.IdentifierToken) {
            if (namespace.hasImport(token.value)) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.PropertyAccessorToken)]);

                var foreignSymbolToken = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

                instance.value = new namespaces.ForeignSymbolReference(namespace, token.value, foreignSymbolToken.value);
            } else {
                instance.value = new namespaces.Symbol(namespace, token.value);
            }
        } else if (token instanceof tokeniser.StringToken || token instanceof tokeniser.NumberToken) {
            instance.value = token.value;
        } else {
            throw new Error("Not implemented");
        }

        return instance;
    }

    static createByValue(value) {
        var instance = new this();

        instance.value = value;

        return instance;
    }

    getContainerTypeValue() {
        var thisScope = this;

        if (this.memoisedContainedTypeValue != null) {
            return this.memoisedContainedTypeValue;
        }

        function getByte(index) {
            var childValue = thisScope.children[index].value;

            if (typeof(childValue) == "boolean") {
                return childValue ? 0x01 : 0x00;
            }

            if (typeof(childValue) == "string") {
                return codeGen.byte(childValue);
            }

            if (typeof(childValue) == "number") {
                return childValue % 256;
            }

            var invalidToken = thisScope.containedValueTokens[index];

            throw new sources.SourceError("Invalid inner value for type literal", invalidToken?.sourceContainer, invalidToken?.location);
        }

        if (this.containerType == "byte") {
            return this.memoisedContainedTypeValue = getByte(0);
        }

        if (this.containerType == "buffer") {
            this.memoisedContainedTypeValue = this.children.map(function(child, index) {
                if (typeof(child.value) == "string") {
                    return codeGen.join(
                        new TextEncoder().encode(child.value)
                    );
                }

                return codeGen.bytes(getByte(index));
            });

            this.bufferLength = this.memoisedContainedTypeValue.reduce((accumulator, value) => accumulator + value.length, 0);

            return this.memoisedContainedTypeValue;
        }

        return null;
    }

    checkSymbolUsage(scope) {
        scope.addSymbol(this.value, true, false, this);

        if (this.value instanceof namespaces.ForeignSymbolReference) {
            var usage = new namespaces.ForeignSymbolUsage(this.value.symbolName, this.value.subjectNamespaceIdentifier);

            usage.readBy.push(this);

            scope.foreignSymbolUses.push(usage);
        }

        super.checkSymbolUsage(scope);
    }

    estimateTruthiness() {
        if (this.containerType == "byte") {
            return this.getContainerTypeValue() != 0;
        }

        if (this.containerType == "buffer") {
            return this.bufferLength > 0;
        }

        if (
            this.value instanceof namespaces.Symbol ||
            this.value instanceof namespaces.ForeignSymbolReference
        ) {
            return this.scope?.getSymbolTruthiness(this.value) ?? null;
        }

        if (this.value == null) {
            return false;
        }

        if (typeof(this.value) == "boolean") {
            return this.value;
        }

        if (typeof(this.value) == "string") {
            return this.value.length > 0;
        }

        if (typeof(this.value) == "number") {
            return this.value != 0;
        }

        return null;
    }

    generateCode(options) {
        if (this.containerType == "byte") {
            return codeGen.bytes(codeGen.vxcTokens.BYTE, this.getContainerTypeValue());
        }

        if (this.containerType == "buffer") {
            var value = this.getContainerTypeValue();

            return codeGen.join(
                codeGen.bytes(codeGen.vxcTokens.BUFFER),
                codeGen.int32(this.bufferLength),
                codeGen.join(...value)
            );
        }

        if (
            this.value instanceof namespaces.Symbol ||
            this.value instanceof namespaces.ForeignSymbolReference
        ) {
            return codeGen.join(
                this.value.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET)
            );
        }

        if (this.value == null) {
            return codeGen.bytes(codeGen.vxcTokens.NULL);
        }

        if (typeof(this.value) == "boolean") {
            return codeGen.boolean(this.value);
        }

        if (typeof(this.value) == "string") {
            return codeGen.string(this.value);
        }

        if (typeof(this.value) == "number") {
            return codeGen.number(this.value);
        }

        throw new Error("Not implemented");
    }
}

export class ObjectNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "object";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "{")
    ];

    propertySymbols = [];
    propertySymbolIsString = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var addedFirstItem = false;

        while (true) {
            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "}")])) {
                break;
            }

            if (addedFirstItem) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)]);
            }

            var retain = !!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "retain")]);

            var propertyToken = this.eat(tokens, [
                new ast.TokenQuery(tokeniser.IdentifierToken),
                new ast.TokenQuery(tokeniser.StringToken)
            ]);

            instance.propertySymbols.push(namespaces.Symbol.generateForProperty(propertyToken.value, retain));
            instance.propertySymbolIsString.push(propertyToken instanceof tokeniser.StringToken);

            this.eat(tokens, [new ast.TokenQuery(tokeniser.PropertyDefinerToken)]);

            instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

            addedFirstItem = true;
        }

        return instance;
    }

    estimateTruthiness() {
        return this.propertySymbols.length > 0;
    }

    generateCode(options) {
        return codeGen.join(
            codeGen.number(0),
            codeGen.systemCall("O"),
            ...this.children.map((child, i) => codeGen.join(
                codeGen.bytes(codeGen.vxcTokens.DUPE),
                child.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.SWAP),
                this.propertySymbolIsString[i] ? codeGen.string(this.propertySymbols[i].name) : this.propertySymbols[i].generateCode(options),
                codeGen.number(3),
                codeGen.systemCall("Os"),
                codeGen.bytes(codeGen.vxcTokens.POP, codeGen.vxcTokens.POP)
            ))
        );
    }
}

export class InstantiationNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`new` expression";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "new")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.instantiateSymbol = new namespaces.Symbol(namespaces.coreNamespace, "instantiate");

        this.eat(tokens);

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")])) {
            instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

            this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")]);
        } else {
            instance.expectChildByMatching(tokens, [ThingNode], namespace);
        }

        instance.expectChildByMatching(tokens, [ConstructorArgumentsNode], namespace);

        return instance;
    }

    checkSymbolUsage(scope) {
        scope.addCoreNamespaceSymbol(this.instantiateSymbol, this);

        super.checkSymbolUsage(scope);
    }

    generateCode(options) {
        return codeGen.join(
            this.children[0].generateCode(options),
            this.children[1].generateCode(options),
            codeGen.number(2),
            this.instantiateSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
        );
    }
};

export class ListNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "list";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "[")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var addedFirstItem = false;

        while (true) {
            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "]")])) {
                break;
            }

            if (addedFirstItem) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)]);
            }

            instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

            addedFirstItem = true;
        }

        return instance;
    }

    estimateTruthiness() {
        return this.children.length > 0;
    }

    generateCode(options) {
        return codeGen.join(
            ...this.children.map((child) => child.generateCode(options)),
            codeGen.number(this.children.length),
            codeGen.systemCall("Lo")
        );
    }
}

export class FunctionParametersNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "parameter list";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "(")
    ];

    parameters = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var addedFirstParameter = false;

        while (true) {
            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")])) {
                break;
            }

            if (addedFirstParameter) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)]);
            }

            instance.parameters.push(
                new namespaces.Symbol(namespace, this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value)
            );

            addedFirstParameter = true;
        }

        return instance;
    }

    checkSymbolUsage(scope) {
        for (var parameter of this.parameters) {
            scope.addSymbol(parameter, false, true);
        }

        super.checkSymbolUsage(scope);
    }

    generateCode(options) {
        return codeGen.join(
            codeGen.number(this.parameters.length),
            codeGen.systemCall("P"),
            ...this.parameters.reverse().map((symbol) => codeGen.join(
                symbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.VAR, codeGen.vxcTokens.POP)
            ))
        );
    }
}

export class FunctionNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "function declaration";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "function")
    ];

    identifierSymbol = null;
    isAnonymous = false;
    skipSymbol = null;
    capturedSymbols = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var identifier = this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

        if (identifier == null) {
            instance.isAnonymous = true;
        }

        instance.identifierSymbol = new namespaces.Symbol(namespace, !instance.isAnonymous ? identifier.value : namespaces.generateSymbolName("anonfn"));
        instance.skipSymbol = new namespaces.Symbol(namespace, "#fn");

        instance.expectChildByMatching(tokens, [FunctionParametersNode], namespace);

        instance.expectChildByMatching(tokens, [statements.StatementBlockNode], namespace);

        return instance;
    }

    checkSymbolUsage(scope) {
        var usage = scope.addSymbol(this.identifierSymbol, false, true);
        var moduleNode = this.findAncestorOfTypes([parser.ModuleNode]);

        usage.truthiness = true;

        var capturedSymbolIds = [];

        // Determine symbols to capture for closure
        for (var expressionNode of this.findDescendantsOfTypes([ThingNode])) {
            var descendant = expressionNode.value;

            if (descendant instanceof namespaces.Symbol) {
                if (!scope.getSymbolById(descendant.id, false, true)) {
                    // If not defined outside of function, then don't capture (defined locally)
                    continue;
                }

                if (scope.findScopeWhereSymbolIsDefined(descendant.id) == moduleNode.scope) {
                    // If global variable, then don't capture (can be accessed anyway)
                    continue;
                }

                if (this.children[0].parameters.find((parameter) => parameter.id == descendant.id)) {
                    // If parameter, then don't capture
                    continue;
                }

                if (capturedSymbolIds.includes(descendant.id)) {
                    continue;
                }

                this.capturedSymbols.push(descendant);
                capturedSymbolIds.push(descendant.id);

                scope.addSymbol(descendant, true, false, this);
            }
        }

        super.checkSymbolUsage(scope, true);
    }

    pruneSymbolUsage() {
        if (this.isAnonymous) {
            return false;
        }

        var usage = this.scope.symbolUses.find((usage) => usage.id == this.identifierSymbol.id);
        var anyMarkedUnread = false;

        if (usage && !usage.everRead) {
            anyMarkedUnread ||= dce.markChildSymbolsAsUnread(this.children[1]);
        }

        return super.pruneSymbolUsage() || anyMarkedUnread;
    }

    estimateTruthiness() {
        return true;
    }

    generateCode(options) {
        if (options.removeDeadCode && !this.isAnonymous) {
            var usage = this.scope.symbolUses.find((usage) => usage.id == this.identifierSymbol.id);
            
            if (usage && !usage.everRead && dce.hasNoEffect(this)) {
                return codeGen.bytes(codeGen.vxcTokens.NULL);
            }
        }

        var symbolCode = this.identifierSymbol.generateCode(options);

        var bodyCode = codeGen.join(
            this.children[0].generateCode(options), // Function parameters
            this.children[1].generateCode(options), // Function statement block
            codeGen.bytes(codeGen.vxcTokens.NULL, codeGen.vxcTokens.RETURN)
        );

        var skipJumpCode = codeGen.join(
            this.skipSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );

        var storageCode = codeGen.join(
            symbolCode,
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(skipJumpCode.length)
        );

        var skipDefinitionCode = codeGen.join(
            this.skipSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(symbolCode.length + storageCode.length + skipJumpCode.length + bodyCode.length)
        );

        var toClosureCode = codeGen.bytes();

        var returnCode = codeGen.join(
            codeGen.bytes(codeGen.vxcTokens.POP),
            symbolCode,
            codeGen.bytes(codeGen.vxcTokens.GET)
        );

        if (this.capturedSymbols.length > 0) {
            toClosureCode = codeGen.join(
                symbolCode,
                codeGen.bytes(codeGen.vxcTokens.GET),
                codeGen.number(0),
                codeGen.systemCall("O"),
                ...this.capturedSymbols.map((symbol) => codeGen.join(
                    codeGen.bytes(codeGen.vxcTokens.DUPE),
                    symbol.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.SWAP),
                    symbol.generateCode(options),
                    codeGen.number(3),
                    codeGen.systemCall("Os"),
                    codeGen.bytes(codeGen.vxcTokens.POP, codeGen.vxcTokens.POP)
                )),
                codeGen.number(2),
                codeGen.systemCall("C"),
                symbolCode,
                codeGen.bytes(codeGen.vxcTokens.SET, codeGen.vxcTokens.POP)
            );
        }

        return codeGen.join(
            skipDefinitionCode,
            symbolCode,
            storageCode,
            skipJumpCode,
            bodyCode,
            toClosureCode,
            returnCode
        );
    }

    describe() {
        var parts = super.describe();

        parts.unshift(`assign to \`${this.identifierSymbol.id}\``);

        if (this.scope != null) {
            var targetUsage = this.scope.symbolUses.find((usage) => usage.id == this.identifierSymbol.id);

            if (targetUsage.everRead) {
                parts.push(`read by ${[...new Set(targetUsage.readBy)].map((reader) => reader != null ? String(reader.id) : "other").join(", ")}`);
            } else {
                parts.push("never read");
            }
        }

        return parts;
    }

    generateContextPath() {
        return `${this.parent.generateContextPath()}->${this.identifierSymbol.id}()`;
    }
}

export class MethodNode extends FunctionNode {
    static HUMAN_READABLE_NAME = "method declaration";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "(")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.identifierSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("anonfn"));
        instance.isAnonymous = true;
        instance.skipSymbol = new namespaces.Symbol(namespace, "#fn");

        instance.expectChildByMatching(tokens, [FunctionParametersNode], namespace);

        instance.expectChildByMatching(tokens, [statements.StatementBlockNode], namespace);

        return instance;
    }
}

export class ClassNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "class declaration";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "class")
    ];

    identifierSymbol = null;
    isAnonymous = false;
    extendsOtherClasses = false;
    setPrototypesSymbol = null;
    propertySymbols = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var identifier = this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

        if (identifier == null) {
            instance.isAnonymous = true;
        }

        instance.identifierSymbol = new namespaces.Symbol(namespace, !instance.isAnonymous ? identifier.value : namespaces.generateSymbolName("anonclass"));        

        if (instance.addChildByMatching(tokens, [ClassExtendsNode], namespace)) {
            instance.extendsOtherClasses = true;
            instance.setPrototypesSymbol = new namespaces.Symbol(namespaces.coreNamespace, "setPrototypes");
        }

        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "{")]);

        while (true) {
            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "}")])) {
                break;
            }

            var retain = !!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "retain")]);
            var propertyToken = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

            instance.propertySymbols.push(namespaces.Symbol.generateForProperty(propertyToken.value, retain));

            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.OperatorToken, "=")])) {
                instance.expectChildByMatching(tokens, [ExpressionNode], namespace);
            } else {
                instance.expectChildByMatching(tokens, [MethodNode], namespace);
            }

            this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.StatementDelimeterToken)]);
        }

        return instance;
    }

    checkSymbolUsage(scope) {
        scope.addSymbol(this.identifierSymbol, false, true);

        if (this.extendsOtherClasses) {
            scope.addCoreNamespaceSymbol(this.setPrototypesSymbol, this);
        }

        super.checkSymbolUsage(scope);
    }

    pruneSymbolUsage() {
        if (this.isAnonymous) {
            return false;
        }

        var usage = this.scope.symbolUses.find((usage) => usage.id == this.identifierSymbol.id);
        var anyMarkedUnread = false;

        if (usage && !usage.everRead) {
            anyMarkedUnread ||= dce.markChildSymbolsAsUnread(this);
        }

        return super.pruneSymbolUsage() || anyMarkedUnread;
    }

    estimateTruthiness() {
        return this.propertySymbols.length > 0;
    }

    generateCode(options) {
        var extendsNode = null;

        if (this.extendsOtherClasses) {
            extendsNode = this.children.shift();
        }

        if (options.removeDeadCode && !this.isAnonymous) {
            var usage = this.scope.symbolUses.find((usage) => usage.id == this.identifierSymbol.id);

            if (usage && !usage.everRead && dce.hasNoEffect(this, this.children.filter((child) => !(child instanceof MethodNode)))) {
                return codeGen.bytes(codeGen.vxcTokens.NULL);
            }
        }

        return codeGen.join(
            codeGen.number(0),
            codeGen.systemCall("O"),
            (!this.isAnonymous || this.findDescendantsOfTypes([SuperNode]).length > 0) ? codeGen.join(
                this.identifierSymbol.generateCode(),
                codeGen.bytes(codeGen.vxcTokens.SET)
            ) : codeGen.bytes(),
            ...this.children.map((child, i) => (
                (
                    !options.removeDeadCode ||
                    namespaces.propertyIsUsed(this.propertySymbols[i]) ||
                    !dce.hasNoEffect(this, [child])
                ) ?
                codeGen.join(
                    codeGen.bytes(codeGen.vxcTokens.DUPE),
                    child.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.SWAP),
                    this.propertySymbols[i].generateCode(options),
                    codeGen.number(3),
                    codeGen.systemCall("Os"),
                    codeGen.bytes(codeGen.vxcTokens.POP, codeGen.vxcTokens.POP)
                ) :
                codeGen.bytes()
            )),
            this.extendsOtherClasses ? codeGen.join(
                codeGen.bytes(codeGen.vxcTokens.DUPE),
                extendsNode.generateCode(options),
                codeGen.number(2),
                this.setPrototypesSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP)
            ) : codeGen.bytes()
        );
    }

    describe() {
        var parts = super.describe();

        parts.unshift(`assign to \`${this.identifierSymbol.id}\``);

        if (this.scope != null) {
            var targetUsage = this.scope.symbolUses.find((usage) => usage.id == this.identifierSymbol.id);

            if (targetUsage.everRead) {
                parts.push(`read by ${[...new Set(targetUsage.readBy)].map((reader) => reader != null ? String(reader.id) : "other").join(", ")}`);
            } else {
                parts.push("never read");
            }
        }

        return parts;
    }

    generateContextPath() {
        return `${this.parent.generateContextPath()}->${this.identifierSymbol.id}{}`;
    }
}

export class ClassExtendsNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`extends` list";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "extends")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        while (true) {
            instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

            if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)])) {
                break;
            }
        }

        return instance;
    }

    generateCode(options) {
        return codeGen.join(
            ...this.children.map((child) => child.generateCode(options)),
            codeGen.number(this.children.length),
            codeGen.systemCall("Lo")
        );
    }
}

export class FunctionArgumentsNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "argument list";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "(")
    ];

    arguments = [];
    spreading = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var addedFirstArgument = false;

        while (true) {
            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")])) {
                break;
            }

            if (addedFirstArgument) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)]);
            }

            instance.spreading.push(!!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.OperatorToken, "...")]));

            instance.arguments.push(
                instance.expectChildByMatching(tokens, [ExpressionNode], namespace)
            );

            addedFirstArgument = true;
        }

        return instance;
    }

    generateCode(options) {
        if (this.spreading.length == 1 && this.spreading[0]) {
            // TODO: Allow spreading multiple args

            return codeGen.join(
                this.children[0].generateCode(options),
                codeGen.number(1),
                codeGen.systemCall("Au")
            );
        }

        return codeGen.join(
            ...this.children.map((child) => child.generateCode(options)),
            codeGen.number(this.children.length)
        );
    }
}

export class ConstructorArgumentsNode extends FunctionArgumentsNode {
    generateCode(options) {
        return codeGen.join(
            ...this.children.map((child) => child.generateCode(options)),
            codeGen.number(this.children.length),
            codeGen.systemCall("Lo")
        );
    }
}

export class FunctionCallNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "function call";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "(")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.pushThisSymbol = new namespaces.Symbol(namespaces.coreNamespace, "pushThis");
        instance.popThisSymbol = new namespaces.Symbol(namespaces.coreNamespace, "popThis");

        instance.expectChildByMatching(tokens, [FunctionArgumentsNode], namespace);

        return instance;
    }

    generateCode(expressionCode, calledAsMethod, options) {
        if (calledAsMethod) {
            return codeGen.join(
                this.children[0].generateCode(options),
                expressionCode,
                codeGen.number(0),
                this.pushThisSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP),
                codeGen.bytes(codeGen.vxcTokens.CALL),
                codeGen.number(0),
                this.popThisSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP)
            );
        }

        return codeGen.join(
            this.children[0].generateCode(options),
            expressionCode,
            codeGen.bytes(codeGen.vxcTokens.CALL)
        );
    }
}

export class IndexAccessorNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "index accessor";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "[")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "]")]);

        return instance;
    }

    generateCode(options) {
        return codeGen.join(
            this.children[0].generateCode(options),
            codeGen.number(2),
            codeGen.systemCall("Tg")
        );
    }

    generateSetterCode(targetCode, valueCode, options) {
        return codeGen.join(
            valueCode,
            targetCode,
            this.children[0].generateCode(options),
            codeGen.number(3),
            codeGen.systemCall("Ts"),
            codeGen.bytes(codeGen.vxcTokens.POP)
        );
    }
}

export class PropertyAccessorNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "property accessor";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.PropertyAccessorToken)
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.getPropertySymbol = new namespaces.Symbol(namespaces.coreNamespace, "getProperty");
        instance.setPropertySymbol = new namespaces.Symbol(namespaces.coreNamespace, "setProperty");

        // TODO: Create a special symbol class for properties so that they can be mangled but are not confined to namespaces
        instance.propertySymbol = namespaces.Symbol.generateForProperty(this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value);

        return instance;
    }

    checkSymbolUsage(scope) {
        scope.addCoreNamespaceSymbol(this.getPropertySymbol, this);
        scope.addCoreNamespaceSymbol(this.setPropertySymbol, this);

        namespaces.propertySymbolUses.push(scope.addSymbol(this.propertySymbol, true, false, this));

        super.checkSymbolUsage(scope);
    }

    generateCode(options) {
        return codeGen.join(
            this.propertySymbol.generateCode(options),
            codeGen.number(2),
            this.getPropertySymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
        );
    }

    generateSetterCode(targetCode, valueCode, options) {
        return codeGen.join(
            targetCode,
            this.propertySymbol.generateCode(options),
            valueCode,
            codeGen.number(3),
            this.setPropertySymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP)
        );
    }
}

export class ExpressionNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "expression";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "syscall"),
        new ast.TokenQuery(tokeniser.KeywordToken, "var"),
        new ast.TokenQuery(tokeniser.BracketToken, "("),
        ...ThisNode.MATCH_QUERIES,
        ...SuperNode.MATCH_QUERIES,
        ...ThingNode.MATCH_QUERIES,
        ...ObjectNode.MATCH_QUERIES,
        ...InstantiationNode.MATCH_QUERIES,
        ...ListNode.MATCH_QUERIES,
        ...FunctionNode.MATCH_QUERIES,
        ...ClassNode.MATCH_QUERIES,
        ...staticMacros.StaticMacroNode.MATCH_QUERIES,
        new ast.TokenQuery(tokeniser.IncrementationOperatorToken),
        new ast.TokenQuery(tokeniser.OperatorToken, "-"),
        new ast.TokenQuery(tokeniser.OperatorToken, "!"),
        new ast.TokenQuery(tokeniser.OperatorToken, "&")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [NullishCoalescingOperatorExpressionNode], namespace);

        return instance;
    }

    estimateTruthiness() {
        return this.children[0]?.estimateTruthiness() ?? null;
    }

    generateCode(options) {
        return codeGen.join(...this.children.map((child) => child.generateCode(options)));
    }
}

export class ExpressionAssignmentNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "assignment expression";

    constructor(targetInstance, isLocal) {
        super();

        this.targetInstance = targetInstance;
        this.isLocal = isLocal;
    }

    static create(tokens, namespace, targetInstance, isLocal = false) {
        var instance = new this(targetInstance, isLocal);

        if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.OperatorToken, "=")])) {
            return instance;
        }

        instance.addChildByMatching(tokens, [ExpressionNode], namespace);

        return instance;
    }

    checkSymbolUsage(scope) {
        var subject = this.targetInstance.children[0];
        var target = this.targetInstance.children.at(-1);

        if (target instanceof PropertyAccessorNode) {
            scope.addSymbol(target.propertySymbol, false, true);

            subject.checkSymbolUsage(scope);
        }

        super.checkSymbolUsage(scope);

        if (subject.value instanceof namespaces.Symbol) {
            var usage = scope.getSymbolById(subject.value.id, this.isLocal);

            usage.everDefined = true;
            usage.truthiness = this.estimateTruthiness();
        }
    }

    pruneSymbolUsage() {
        var usage = this.scope.symbolUses.find((usage) => usage.id == this.targetInstance.children.at(-1)?.value?.id);
        var anyMarkedUnread = false;

        if (this.children.length > 0 && usage && !usage.everRead && dce.hasNoEffect(this, this.children.length > 0 ? [this.children[0]] : [])) {
            anyMarkedUnread ||= dce.markChildSymbolsAsUnread(this.children[0]);
        }

        return super.pruneSymbolUsage() || anyMarkedUnread;
    }

    estimateTruthiness() {
        if (this.children.length > 0) {
            return this.children[0].estimateTruthiness();
        }

        return null;
    }

    generateCode(options) {
        var valueCode = this.children.length > 0 ? this.children[0].generateCode(options) : codeGen.bytes(codeGen.vxcTokens.NULL);
        var target = this.targetInstance.children.pop();

        if (this.targetInstance.children.length > 0) {
            if (target instanceof IndexAccessorNode || target instanceof PropertyAccessorNode) {
                return target.generateSetterCode(
                    this.targetInstance.generateCode(options),
                    valueCode,
                    options
                );
            }

            // TODO: Specify token
            throw new sources.SourceError("Cannot set a value for this type of accessor");
        }

        if (!(target instanceof ThisNode || target instanceof ThingNode)) {
            throw new Error("Trap: setter target is not a `ThisNode` or `ThingNode`");
        }

        if (!(target.value instanceof namespaces.Symbol)) {
            // TODO: Specify token
            throw new sources.SourceError("Cannot set a value literal (expected a variable name)");
        }

        if (options.removeDeadCode) {
            var usage = this.scope.symbolUses.find((usage) => usage.id == target.value.id);

            if (usage && !usage.everRead) {
                return dce.hasNoEffect(this, this.children.length > 0 ? [this.children[0]] : []) ? codeGen.bytes(codeGen.vxcTokens.NULL) : valueCode;
            }
        }

        return codeGen.join(
            valueCode,
            target.value.generateCode(options),
            codeGen.bytes(this.isLocal ? codeGen.vxcTokens.VAR : codeGen.vxcTokens.SET)
        );
    }

    describe() {
        var parts = super.describe();
        var target = this.targetInstance.children[0];

        if (target.value instanceof namespaces.Symbol) {
            parts.unshift(`assign to \`${target.value.id}\``);

            if (this.scope != null) {
                var targetUsage = this.scope.symbolUses.find((usage) => usage.id == target.value.id);

                if (targetUsage?.everRead) {
                    parts.push(`read by ${[...new Set(targetUsage.readBy)].map((reader) => reader != null ? String(reader.id) : "other").join(", ")}`);
                } else {
                    parts.push("never read");
                }
            }
        }

        return parts;
    }

    generateContextPath() {
        return `${this.parent.generateContextPath()}->${this.targetInstance.children[0]?.value?.id || "[var]"}=`;
    }
}

export class ExpressionLeafNode extends ExpressionNode {
    static maybeAddAccessors(instance, tokens, namespace) {
        while (true) {
            if (instance.addChildByMatching(tokens, [FunctionCallNode, IndexAccessorNode, PropertyAccessorNode], namespace)) {
                continue;
            }

            break;
        }
    }

    static create(tokens, namespace) {
        var instance = new this();
        var assigningToLocalVariable = false;

        if (instance.addChildByMatching(tokens, [
            UnaryPrefixIncrementationOperatorExpressionNode,
            UnaryNegativeOperatorExpressionNode,
            UnaryNotOperatorExpressionNode,
            CopyOperatorExpressionNode
        ], namespace)) {
            return instance;
        }

        if (this.want(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "syscall")])) {
            return SystemCallNode.create(tokens, namespace);
        }

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "var")])) {
            assigningToLocalVariable = true;
        }

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")])) {
            instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

            this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")]);

            this.maybeAddAccessors(instance, tokens, namespace);
        } else {
            instance.expectChildByMatching(tokens, [ExpressionThingNode], namespace);

            instance.children = instance.children[0].children;
        }

        if (this.want(tokens, [new ast.TokenQuery(tokeniser.OperatorToken, "=")])) {
            return ExpressionAssignmentNode.create(tokens, namespace, instance, assigningToLocalVariable);
        }

        if (assigningToLocalVariable) {
            return ExpressionAssignmentNode.create(tokens, namespace, instance, true);
        }

        return instance;
    }

    checkSymbolUsage(scope) {
        for (var child of this.children) {
            if (child instanceof FunctionCallNode) {
                scope.addCoreNamespaceSymbol(child.pushThisSymbol, this);
                scope.addCoreNamespaceSymbol(child.popThisSymbol, this);
            }
        }

        super.checkSymbolUsage(scope);
    }

    estimateTruthiness() {
        return this.children.at(-1).estimateTruthiness();
    }

    generateCode(options) {
        var currentCode = codeGen.bytes();
        var lastNodeWasPropertyAccessor = false;

        for (var child of this.children) {
            if (child instanceof FunctionCallNode) {
                // Function calls need to push the arguments passed to the function first before the function thing is pushed on
                currentCode = child.generateCode(currentCode, lastNodeWasPropertyAccessor, options);

                continue;
            }

            currentCode = codeGen.join(currentCode, child.generateCode(options));
            lastNodeWasPropertyAccessor = child instanceof PropertyAccessorNode;
        }

        return currentCode;
    }
}

export class ExpressionThingNode extends ExpressionLeafNode {
    static MATCH_QUERIES = [
        ...ThisNode.MATCH_QUERIES,
        ...SuperNode.MATCH_QUERIES,
        ...ThingNode.MATCH_QUERIES,
        ...ObjectNode.MATCH_QUERIES,
        ...InstantiationNode.MATCH_QUERIES,
        ...ListNode.MATCH_QUERIES,
        ...FunctionNode.MATCH_QUERIES,
        ...ClassNode.MATCH_QUERIES,
        ...staticMacros.StaticMacroNode.MATCH_QUERIES
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [
            ThisNode,
            SuperNode,
            ThingNode,
            ObjectNode,
            InstantiationNode,
            ListNode,
            FunctionNode,
            ClassNode,
            staticMacros.StaticMacroNode
        ], namespace);

        this.maybeAddAccessors(instance, tokens, namespace);

        return instance;
    }
}

export class UnaryPrefixOperatorExpressionNode extends ExpressionNode {
    static MATCH_QUERIES = [];
    static OPERATOR_CODE = null;

    operatorToken = null;

    static create(tokens, namespace) {
        var instance = new this();

        instance.operatorToken = this.eat(tokens);

        instance.expectChildByMatching(tokens, [ExpressionLeafNode], namespace);

        return instance;
    }

    generateCode(options) {
        return codeGen.join(this.children[0].generateCode(options), this.constructor.OPERATOR_CODE);
    }
}

export class UnaryPrefixIncrementationOperatorExpressionNode extends UnaryPrefixOperatorExpressionNode {
    static MATCH_QUERIES = [new ast.TokenQuery(tokeniser.IncrementationOperatorToken)];

    estimateTruthiness() {
        return null;
    }

    generateCode(options) {
        return codeGen.join(
            this.children[0].generateCode(options),
            codeGen.number(1),
            codeGen.systemCall(this.operatorToken.value)
        );
    }
}

export class UnaryNegativeOperatorExpressionNode extends UnaryPrefixOperatorExpressionNode {
    static MATCH_QUERIES = [new ast.TokenQuery(tokeniser.OperatorToken, "-")];

    static OPERATOR_CODE = codeGen.join(
        codeGen.number(1),
        codeGen.systemCall("-x")
    );

    estimateTruthiness() {
        return this.children[0].estimateTruthiness();
    }
}

export class UnaryNotOperatorExpressionNode extends UnaryPrefixOperatorExpressionNode {
    static MATCH_QUERIES = [new ast.TokenQuery(tokeniser.OperatorToken, "!")];

    static OPERATOR_CODE = codeGen.bytes(codeGen.vxcTokens.NOT);

    estimateTruthiness() {
        var valueTruthiness = this.children[0].estimateTruthiness();

        if (valueTruthiness == null) {
            return null;
        }

        return !valueTruthiness;
    }
}

export class CopyOperatorExpressionNode extends UnaryPrefixOperatorExpressionNode {
    static MATCH_QUERIES = [new ast.TokenQuery(tokeniser.OperatorToken, "&")];

    static OPERATOR_CODE = codeGen.join(
        codeGen.number(1),
        codeGen.systemCall("Tc")
    );

    estimateTruthiness() {
        return this.children[0].estimateTruthiness();
    }
}

export class UnarySuffixOperatorExpressionNode extends ExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [];
    static OPERATOR_CODE = null;
    static CHILD_EXPRESSION_NODE_CLASS = ExpressionLeafNode;

    operatorToken = null;

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [this.CHILD_EXPRESSION_NODE_CLASS], namespace);

        instance.operatorToken = this.maybeEat(tokens, this.OPERATOR_TOKEN_QUERIES);

        if (!instance.operatorToken) {
            return instance.children[0];
        }

        return instance;
    }

    generateCode(options) {
        return codeGen.join(this.children[0].generateCode(options), this.constructor.OPERATOR_CODE);
    }
}

export class UnarySuffixIncrementationOperatorExpressionNode extends UnarySuffixOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [new ast.TokenQuery(tokeniser.IncrementationOperatorToken)];
    static CHILD_EXPRESSION_NODE_CLASS = ExpressionLeafNode;

    estimateTruthiness() {
        return null;
    }

    generateCode(options) {
        return codeGen.join(
            this.children[0].generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.COPY, codeGen.vxcTokens.SWAP),
            codeGen.number(1),
            codeGen.systemCall(this.operatorToken.value),
            codeGen.bytes(codeGen.vxcTokens.POP)
        );
    }
}

export class BinaryOperatorExpressionNode extends ExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [];
    static OPERATOR_CODE = {};
    static CHILD_EXPRESSION_NODE_CLASS = UnarySuffixOperatorExpressionNode;

    operatorTokens = [];

    static create(tokens, namespace) {
        var instance = new this();

        while (true) {
            instance.expectChildByMatching(tokens, [this.CHILD_EXPRESSION_NODE_CLASS], namespace);

            var operatorToken = this.maybeEat(tokens, this.OPERATOR_TOKEN_QUERIES);

            if (!operatorToken) {
                break;
            }

            instance.operatorTokens.push(operatorToken);
        }

        if (instance.children.length == 1) {
            return instance.children[0];
        }

        return instance;
    }

    generateCode(options) {
        var currentCode = this.children[0].generateCode(options);

        for (var i = 1; i < this.children.length; i++) {
            var child = this.children[i];
            var operator = this.operatorTokens[i - 1];

            currentCode = codeGen.join(
                currentCode,
                child.generateCode(options),
                this.constructor.OPERATOR_CODE[operator.value]
            );
        }

        return currentCode;
    }
}

export class MultiplicationDivisionOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "*"),
        new ast.TokenQuery(tokeniser.OperatorToken, "/"),
        new ast.TokenQuery(tokeniser.OperatorToken, "%")
    ];

    static OPERATOR_CODE = {
        "*": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("*")
        ),
        "/": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("/")
        ),
        "%": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("%")
        )
    };

    static CHILD_EXPRESSION_NODE_CLASS = UnarySuffixIncrementationOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class AdditionSubtractionOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "+"),
        new ast.TokenQuery(tokeniser.OperatorToken, "-")
    ];

    static OPERATOR_CODE = {
        "+": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("+")
        ),
        "-": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("-")
        )
    };

    static CHILD_EXPRESSION_NODE_CLASS = MultiplicationDivisionOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class EqualityOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "<="),
        new ast.TokenQuery(tokeniser.OperatorToken, "<"),
        new ast.TokenQuery(tokeniser.OperatorToken, ">="),
        new ast.TokenQuery(tokeniser.OperatorToken, ">"),
        new ast.TokenQuery(tokeniser.OperatorToken, "!="),
        new ast.TokenQuery(tokeniser.OperatorToken, "==")
    ];

    static OPERATOR_CODE = {
        "<=": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("<=")
        ),
        "<": codeGen.bytes(codeGen.vxcTokens.LESS_THAN),
        ">=": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall(">=")
        ),
        ">": codeGen.bytes(codeGen.vxcTokens.GREATER_THAN),
        "!=": codeGen.bytes(codeGen.vxcTokens.EQUAL, codeGen.vxcTokens.NOT),
        "==": codeGen.bytes(codeGen.vxcTokens.EQUAL)
    };

    static CHILD_EXPRESSION_NODE_CLASS = AdditionSubtractionOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class LogicalEagerAndOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "&&&")
    ];

    static OPERATOR_CODE = {
        "&&&": codeGen.bytes(codeGen.vxcTokens.AND)
    };

    static CHILD_EXPRESSION_NODE_CLASS = EqualityOperatorExpressionNode;

    estimateTruthiness() {
        var anyUnknown = false;

        for (var child of this.children) {
            var truthiness = child.estimateTruthiness();

            if (truthiness == null) {
                anyUnknown = true;
            }

            if (truthiness == false) {
                return false;
            }
        }

        return anyUnknown ? null : true;
    }
}

export class LogicalEagerOrOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "|||")
    ];

    static OPERATOR_CODE = {
        "|||": codeGen.bytes(codeGen.vxcTokens.OR)
    };

    static CHILD_EXPRESSION_NODE_CLASS = LogicalEagerAndOperatorExpressionNode;

    estimateTruthiness() {
        var anyUnknown = false;

        for (var child of this.children) {
            var truthiness = child.estimateTruthiness();

            if (truthiness == null) {
                anyUnknown = true;
            }

            if (truthiness == true) {
                return true;
            }
        }

        return anyUnknown ? null : false;
    }
}

export class LogicalShortCircuitingAndOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "&&")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = LogicalEagerOrOperatorExpressionNode;

    static create(tokens, namespace) {
        var instance = super.create(tokens, namespace);

        instance.skipSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("and_skip"));

        return instance;
    }

    estimateTruthiness() {
        var anyUnknown = false;

        for (var child of this.children) {
            var truthiness = child.estimateTruthiness();

            if (truthiness == null) {
                anyUnknown = true;
            }

            if (truthiness == false) {
                return false;
            }
        }

        return anyUnknown ? null : true;
    }

    generateCode(options) {
        if (this.children.length == 1) {
            return this.children[0].generateCode(options);
        }

        var allChildrenCode = this.children.map((child) => child.generateCode(options));
        var skipSymbolCode = this.skipSymbol.generateCode(options);
        var currentCode = allChildrenCode.pop();

        while (allChildrenCode.length > 0) {
            var nextChildCode = allChildrenCode.pop();

            currentCode = codeGen.join(
                nextChildCode,
                codeGen.bytes(codeGen.vxcTokens.DUPE),
                codeGen.bytes(codeGen.vxcTokens.NOT),
                skipSymbolCode,
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY),
                codeGen.bytes(codeGen.vxcTokens.POP),
                currentCode
            );
        }

        currentCode = codeGen.join(
            skipSymbolCode,
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(currentCode.length),
            currentCode
        );

        return currentCode;
    }
}

export class LogicalShortCircuitingOrOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "||")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = LogicalShortCircuitingAndOperatorExpressionNode;

    static create(tokens, namespace) {
        var instance = super.create(tokens, namespace);

        instance.skipSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("or_skip"));

        return instance;
    }

    estimateTruthiness() {
        var anyUnknown = false;

        for (var child of this.children) {
            var truthiness = child.estimateTruthiness();

            if (truthiness == null) {
                anyUnknown = true;
            }

            if (truthiness == true) {
                return true;
            }
        }

        return anyUnknown ? null : false;
    }

    generateCode(options) {
        if (this.children.length == 1) {
            return this.children[0].generateCode(options);
        }

        var allChildrenCode = this.children.map((child) => child.generateCode(options));
        var skipSymbolCode = this.skipSymbol.generateCode(options);
        var currentCode = allChildrenCode.pop();

        while (allChildrenCode.length > 0) {
            var nextChildCode = allChildrenCode.pop();

            currentCode = codeGen.join(
                nextChildCode,
                codeGen.bytes(codeGen.vxcTokens.DUPE),
                skipSymbolCode,
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY),
                codeGen.bytes(codeGen.vxcTokens.POP),
                currentCode
            );
        }

        currentCode = codeGen.join(
            skipSymbolCode,
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(currentCode.length),
            currentCode
        );

        return currentCode;
    }
}

export class NullishCoalescingOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "??")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = LogicalShortCircuitingOrOperatorExpressionNode;

    static create(tokens, namespace) {
        var instance = super.create(tokens, namespace);

        instance.skipSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("nullish_skip"));

        return instance;
    }

    estimateTruthiness() {
        var anyUnknown = false;

        for (var child of this.children) {
            var truthiness = child.estimateTruthiness();

            if (truthiness == null) {
                anyUnknown = true;
            }

            if (truthiness == true) {
                return true;
            }
        }

        return anyUnknown ? null : false;
    }

    generateCode(options) {
        if (this.children.length == 1) {
            return this.children[0].generateCode(options);
        }

        var allChildrenCode = this.children.map((child) => child.generateCode(options));
        var skipSymbolCode = this.skipSymbol.generateCode(options);
        var currentCode = allChildrenCode.pop();

        while (allChildrenCode.length > 0) {
            var nextChildCode = allChildrenCode.pop();

            currentCode = codeGen.join(
                nextChildCode,
                codeGen.bytes(codeGen.vxcTokens.DUPE, codeGen.vxcTokens.NULL, codeGen.vxcTokens.EQUAL, codeGen.vxcTokens.NOT),
                skipSymbolCode,
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY),
                codeGen.bytes(codeGen.vxcTokens.POP),
                currentCode
            );
        }

        currentCode = codeGen.join(
            skipSymbolCode,
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(currentCode.length),
            currentCode
        );

        return currentCode;
    }
}

export class SystemCallNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "system call";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "syscall")
    ];

    value = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.value = new namespaces.SystemCall(namespace, this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value);

        instance.expectChildByMatching(tokens, [FunctionArgumentsNode], namespace);

        return instance;
    }

    generateCode(options) {
        return codeGen.join(
            this.children[0].generateCode(options),
            this.value.generateCode(options)
        );
    }
}