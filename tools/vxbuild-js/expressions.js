import * as sources from "./sources.js";
import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";
import * as dce from "./dce.js";
import * as parser from "./parser.js";
import * as statements from "./statements.js";
import * as staticMacros from "./staticmacros.js";

export const ALL_BINARY_OPERATOR_CODE = {
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
    ),
    "+": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall("+")
    ),
    "-": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall("-")
    ),
    "<<": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall("<<")
    ),
    ">>": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall(">>")
    ),
    ">>>": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall(">>>")
    ),
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
    "!==": codeGen.bytes(codeGen.vxcTokens.IDENTICAL, codeGen.vxcTokens.NOT),
    "!=": codeGen.bytes(codeGen.vxcTokens.EQUAL, codeGen.vxcTokens.NOT),
    "===": codeGen.bytes(codeGen.vxcTokens.IDENTICAL),
    "==": codeGen.bytes(codeGen.vxcTokens.EQUAL),
    "&": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall("&")
    ),
    "^": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall("^")
    ),
    "|": codeGen.join(
        codeGen.number(2),
        codeGen.systemCall("|")
    ),
    "&&&": codeGen.bytes(codeGen.vxcTokens.AND),
    "&&": codeGen.bytes(codeGen.vxcTokens.AND),
    "|||": codeGen.bytes(codeGen.vxcTokens.OR),
    "||": codeGen.bytes(codeGen.vxcTokens.OR),
    "??": codeGen.bytes(codeGen.vxcTokens.SWAP, codeGen.vxcTokens.POP),
    "is": codeGen.join(
        codeGen.boolean(false),
        codeGen.number(3),
        codeGen.systemCall("Ti")
    ),
    "inherits": codeGen.join(
        codeGen.boolean(true),
        codeGen.number(3),
        codeGen.systemCall("Ti")
    )
};

export const ALL_BINARY_OPEATOR_SKIP_CODE = {
    "&&": codeGen.bytes(codeGen.vxcTokens.NOT),
    "||": codeGen.bytes(),
    "??": codeGen.bytes(codeGen.vxcTokens.NULL, codeGen.vxcTokens.EQUAL, codeGen.vxcTokens.NOT)
};

export class ThisNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`this`";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "this")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        return instance;
    }

    generateCode(options) {
        return codeGen.bytes(codeGen.vxcTokens.THIS);
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
        new ast.TokenQuery(tokeniser.TypeNameToken),
        new ast.TokenQuery(tokeniser.IdentifierToken),
        new ast.TokenQuery(tokeniser.StringToken),
        new ast.TokenQuery(tokeniser.NumberToken)
    ];

    value = null;
    containedValueTokens = [];
    containerType = null;
    memoisedContainedTypeValue = null;
    bufferLength = null;
    enumIdentifier = null;
    enumEntryIdentifier = null;
    enumNamespace = null;

    static create(tokens, namespace) {
        var instance = new this();

        var token = this.eat(tokens);

        if (token instanceof tokeniser.AtomToken) {
            instance.value = {
                "true": true,
                "false": false,
                "null": null,
                "infinity": Infinity,
                "nan": NaN
            }[token.value] ?? null;
        } else if (token instanceof tokeniser.TypeNameToken) {
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
                throw new sources.SourceError("Type literal for this type does not use this syntax", token?.sourceContainer, token?.location);
            }
        } else if (token instanceof tokeniser.IdentifierToken) {
            if (namespace.hasImport(token.value)) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.PropertyAccessorToken)]);

                var foreignSymbolToken = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

                instance.value = new namespaces.ForeignSymbolReference(
                    namespace,
                    token.value,
                    foreignSymbolToken.value,
                    !!this.want(tokens, [new ast.TokenQuery(tokeniser.PropertyAccessorToken)])
                );
            } else if (token.value in namespace.enums) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.PropertyAccessorToken)]);

                var entryToken = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

                instance.value = namespace.getEnumValue(token.value, entryToken.value);
                instance.enumIdentifier = token.value;
                instance.enumEntryIdentifier = entryToken.value;
                instance.enumNamespace = namespace;
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
            var generatedCode = this.value.generateCode(options);

            if (this.value instanceof namespaces.ForeignSymbolReference && this.value.enum) {
                return generatedCode;
            }

            return codeGen.join(
                generatedCode,
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
            if (this.enumIdentifier != null) {
                this.enumNamespace.markEnumAsUsed(this.enumIdentifier, this.enumEntryIdentifier);
            }

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
    restAtIndex = null;

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

            var restToken = this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.OperatorToken, "...")]);

            if (restToken) {
                if (instance.restAtIndex != null) {
                    throw new sources.SourceError("Only one rest parameter (...) is allowed in a parameter list", spreadingToken?.sourceContainer, spreadingToken?.location);
                }

                instance.restAtIndex = instance.parameters.length;
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
        if (this.restAtIndex != null) {
            var currentCode = codeGen.join(
                codeGen.systemCall("Lo"),
                this.parameters[this.restAtIndex].generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.VAR)
            );

            var remainingParameters = [...this.parameters];

            for (var i = 0; i < this.restAtIndex; i++) {
                currentCode = codeGen.join(
                    currentCode,
                    codeGen.bytes(codeGen.vxcTokens.DUPE, codeGen.vxcTokens.DUPE),
                    codeGen.number(0),
                    codeGen.number(2),
                    codeGen.systemCall("Lg"),
                    remainingParameters.shift().generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.VAR, codeGen.vxcTokens.POP),
                    codeGen.number(0),
                    codeGen.number(2),
                    codeGen.systemCall("Lr"),
                    codeGen.bytes(codeGen.vxcTokens.POP)
                );
            }

            for (var i = 0; i < this.parameters.length - this.restAtIndex - 1; i++) {
                currentCode = codeGen.join(
                    currentCode,
                    codeGen.bytes(codeGen.vxcTokens.DUPE),
                    codeGen.number(1),
                    codeGen.systemCall("Lp"),
                    remainingParameters.pop().generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.VAR, codeGen.vxcTokens.POP)
                );
            }

            return codeGen.join(currentCode, codeGen.bytes(codeGen.vxcTokens.POP));
        }

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
    propertyDescriptorTypes = [];

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

            var retain = false;
            var descriptorType = null;
            var propertyToken;

            while (true) {
                if (!retain && this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "retain")])) {
                    retain = true;

                    continue;
                }

                if (descriptorType == null) {
                    var descriptorTypeToken = this.maybeEat(tokens, [
                        new ast.TokenQuery(tokeniser.KeywordToken, "get"),
                        new ast.TokenQuery(tokeniser.KeywordToken, "set")
                    ]);

                    if (descriptorTypeToken) {
                        descriptorType = descriptorTypeToken.value;

                        continue;
                    }
                }

                propertyToken = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

                break;
            }

            instance.propertySymbols.push(namespaces.Symbol.generateForProperty(propertyToken.value, retain));
            instance.propertyDescriptorTypes.push(descriptorType);

            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.AssignmentOperatorToken, "=")])) {
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
                    (this.propertyDescriptorTypes[i] == "set" || !dce.hasNoEffect(this, [child]))
                ) ?
                codeGen.join(
                    codeGen.bytes(codeGen.vxcTokens.DUPE),
                    child.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.SWAP),
                    this.propertySymbols[i].generateCode(options),
                    codeGen.number(3),
                    codeGen.systemCall({
                        "get": "Osg",
                        "set": "Oss"
                    }[this.propertyDescriptorTypes[i]] || "Os"),
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
            return codeGen.join(
                this.children[0].generateCode(options),
                codeGen.number(1),
                codeGen.systemCall("Au")
            );
        }

        if (this.spreading.find((isSpreading) => isSpreading)) {
            var currentCode = codeGen.bytes();

            for (var i = 0; i < this.children.length; i++) {
                currentCode = codeGen.join(
                    currentCode,
                    this.children[i].generateCode(options),
                    this.spreading[i] ? codeGen.bytes() : codeGen.join(
                        codeGen.number(1),
                        codeGen.systemCall("Lo")
                    ),
                    codeGen.number(2),
                    codeGen.systemCall("Lc")
                );
            }

            return codeGen.join(
                codeGen.number(0),
                codeGen.systemCall("L"),
                currentCode,
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
        if (this.spreading.length == 1 && this.spreading[0]) {
            return this.children[0].generateCode(options);
        }

        if (this.spreading.find((isSpreading) => isSpreading)) {
            var currentCode = codeGen.bytes();

            for (var i = 0; i < this.children.length; i++) {
                currentCode = codeGen.join(
                    currentCode,
                    this.children[i].generateCode(options),
                    this.spreading[i] ? codeGen.bytes() : codeGen.join(
                        codeGen.number(1),
                        codeGen.systemCall("Lo")
                    ),
                    codeGen.number(2),
                    codeGen.systemCall("Lc")
                );
            }

            return codeGen.join(
                codeGen.number(0),
                codeGen.systemCall("L"),
                currentCode
            );
        }

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

        instance.expectChildByMatching(tokens, [FunctionArgumentsNode], namespace);

        return instance;
    }

    generateCode(expressionCode, calledAsMethod, options) {
        if (calledAsMethod) {
            return codeGen.join(
                this.children[0].generateCode(options),
                expressionCode,
                codeGen.number(0),
                codeGen.systemCall("Mu"),
                codeGen.bytes(codeGen.vxcTokens.POP),
                codeGen.bytes(codeGen.vxcTokens.CALL),
                codeGen.number(0),
                codeGen.systemCall("Mp"),
                codeGen.bytes(codeGen.vxcTokens.POP)
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

    generateOperatorSetterCode(targetCode, valueCode, operatorCode, skipConditionCode, skipSymbol, options) {
        if (skipConditionCode == null) {
            return codeGen.join(
                targetCode,
                this.children[0].generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.DUPE),
                codeGen.number(2),
                codeGen.bytes(codeGen.vxcTokens.OVER, codeGen.vxcTokens.SWAP),
                codeGen.number(2),
                codeGen.systemCall("Tg"),
                valueCode,
                operatorCode,
                codeGen.number(2),
                codeGen.bytes(codeGen.vxcTokens.OVER),
                codeGen.number(2),
                codeGen.bytes(codeGen.vxcTokens.OVER),
                codeGen.number(3),
                codeGen.systemCall("Ts"),
                codeGen.bytes(codeGen.vxcTokens.POP, codeGen.vxcTokens.SWAP, codeGen.vxcTokens.POP, codeGen.vxcTokens.SWAP, codeGen.vxcTokens.POP)
            );
        }

        var skipConditionCheckCode = codeGen.join(
            targetCode,
            this.children[0].generateCode(options),
            codeGen.number(2),
            codeGen.systemCall("Tg"),
            codeGen.bytes(codeGen.vxcTokens.DUPE),
            skipConditionCode,
            skipSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY)
        );

        var operatorAssignmentCode = codeGen.join(
            valueCode,
            operatorCode,
            targetCode,
            this.children[0].generateCode(options),
            codeGen.number(3),
            codeGen.systemCall("Ts"),
            codeGen.bytes(codeGen.vxcTokens.POP)
        );

        var skipAssignmentDefinitionCode = codeGen.join(
            skipSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(skipConditionCheckCode.length + operatorAssignmentCode.length)
        );

        return codeGen.join(
            skipAssignmentDefinitionCode,
            skipConditionCheckCode,
            operatorAssignmentCode
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
        var siblings = this.parent.children;
        var previousSibling = siblings[siblings.indexOf(this) - 1];

        if (
            previousSibling instanceof ThingNode &&
            previousSibling.value instanceof namespaces.ForeignSymbolReference &&
            previousSibling.value.enum != null
        ) {
            var enumValue = previousSibling.value.enum[this.propertySymbol.name] ?? null;

            previousSibling.value.symbol.namespace.markEnumAsUsed(previousSibling.value.enumIdentifier, this.propertySymbol.name);

            return enumValue != null ? codeGen.number(enumValue) : codeGen.bytes(codeGen.vxcTokens.NULL);
        }

        return codeGen.join(
            this.propertySymbol.generateCode(options),
            codeGen.number(2),
            this.getPropertySymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
        );
    }

    generateSetterCode(targetCode, valueCode, options) {
        if (targetCode.length == 0) {
            // TODO: Specify token
            throw new sources.SourceError("Cannot set an enum entry");
        }

        return codeGen.join(
            targetCode,
            this.propertySymbol.generateCode(options),
            valueCode,
            codeGen.number(3),
            this.setPropertySymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP)
        );
    }

    generateOperatorSetterCode(targetCode, valueCode, operatorCode, skipConditionCode, skipSymbol, options) {
        if (skipConditionCode == null) {
            return codeGen.join(
                targetCode,
                this.propertySymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.DUPE),
                codeGen.number(2),
                codeGen.bytes(codeGen.vxcTokens.OVER, codeGen.vxcTokens.SWAP),
                codeGen.number(2),
                this.getPropertySymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL),
                valueCode,
                operatorCode,
                codeGen.number(3),
                this.setPropertySymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP)
            );
        }

        var skipConditionCheckCode = codeGen.join(
            targetCode,
            this.propertySymbol.generateCode(options),
            codeGen.number(2),
            this.getPropertySymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL),
            codeGen.bytes(codeGen.vxcTokens.DUPE),
            skipConditionCode,
            skipSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY)
        );

        var operatorAssignmentCode = codeGen.join(
            valueCode,
            operatorCode,
            targetCode,
            this.propertySymbol.generateCode(options),
            codeGen.number(2),
            codeGen.bytes(codeGen.vxcTokens.OVER),
            codeGen.number(3),
            this.setPropertySymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP, codeGen.vxcTokens.POP)
        );

        var skipAssignmentDefinitionCode = codeGen.join(
            skipSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(skipConditionCheckCode.length + operatorAssignmentCode.length)
        );

        return codeGen.join(
            skipAssignmentDefinitionCode,
            skipConditionCheckCode,
            operatorAssignmentCode
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
        new ast.TokenQuery(tokeniser.OperatorToken, "~"),
        new ast.TokenQuery(tokeniser.OperatorToken, "!"),
        new ast.TokenQuery(tokeniser.OperatorToken, "&")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [TernaryOperatorExpressionNode], namespace);

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

    static OPERATOR_ASSIGNMENT_CODE = ALL_BINARY_OPERATOR_CODE;
    static OPERATOR_ASSIGNMENT_SKIP_CODE = ALL_BINARY_OPEATOR_SKIP_CODE;

    static OPERATOR_ASSIGNMENT_MAP = {
        "*=": "*",
        "/=": "/",
        "%=": "%",
        "+=": "+",
        "-=": "-",
        "&&&=": "&&&",
        "|||=": "|||",
        "&&=": "&&",
        "||=": "||",
        "??=": "??"
    };

    operatorAssignment = null;
    skipSymbol = null;

    constructor(targetInstance, isLocal) {
        super();

        this.targetInstance = targetInstance;
        this.isLocal = isLocal;
    }

    static create(tokens, namespace, targetInstance, isLocal = false) {
        var instance = new this(targetInstance, isLocal);

        var operator = this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.AssignmentOperatorToken)]);

        if (!operator) {
            return instance;
        }

        instance.operatorAssignment = this.OPERATOR_ASSIGNMENT_MAP[operator.value] || null;

        if (instance.operatorAssignment) {
            instance.skipSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("assign_skip"));
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
        if (this.operatorAssignment == null && this.children.length > 0) {
            return this.children[0].estimateTruthiness();
        }

        return null;
    }

    generateCode(options) {
        var valueCode = this.children.length > 0 ? this.children[0].generateCode(options) : codeGen.bytes(codeGen.vxcTokens.NULL);
        var target = this.targetInstance.children.pop();
        var operatorCode = this.constructor.OPERATOR_ASSIGNMENT_CODE[this.operatorAssignment] || null;
        var skipConditionCode = this.constructor.OPERATOR_ASSIGNMENT_SKIP_CODE[this.operatorAssignment] || null;

        if (this.targetInstance.children.length > 0) {
            if (target instanceof IndexAccessorNode || target instanceof PropertyAccessorNode) {
                if (operatorCode != null) {
                    return target.generateOperatorSetterCode(
                        this.targetInstance.generateCode(options),
                        valueCode,
                        operatorCode,
                        skipConditionCode,
                        this.skipSymbol,
                        options
                    );
                }

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
            throw new sources.SourceError("Cannot set a value literal or enum entry (expected a variable name)");
        }

        if (options.removeDeadCode) {
            var usage = this.scope.symbolUses.find((usage) => usage.id == target.value.id);

            if (usage && !usage.everRead) {
                return dce.hasNoEffect(this, this.children.length > 0 ? [this.children[0]] : []) ? codeGen.bytes(codeGen.vxcTokens.NULL) : valueCode;
            }
        }

        if (operatorCode != null) {
            if (skipConditionCode == null) {
                return codeGen.join(
                    target.value.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.DUPE, codeGen.vxcTokens.GET),
                    valueCode,
                    operatorCode,
                    codeGen.bytes(codeGen.vxcTokens.SWAP),
                    codeGen.bytes(this.isLocal ? codeGen.vxcTokens.VAR : codeGen.vxcTokens.SET)
                );
            }

            var skipConditionCheckCode = codeGen.join(
                target.value.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.DUPE),
                skipConditionCode,
                this.skipSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY)
            );

            var operatorAssignmentCode = codeGen.join(
                valueCode,
                operatorCode,
                target.value.generateCode(options),
                codeGen.bytes(this.isLocal ? codeGen.vxcTokens.VAR : codeGen.vxcTokens.SET)
            );

            var skipAssignmentDefinitionCode = codeGen.join(
                this.skipSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
                codeGen.int32(skipConditionCheckCode.length + operatorAssignmentCode.length)
            );

            return codeGen.join(
                skipAssignmentDefinitionCode,
                skipConditionCheckCode,
                operatorAssignmentCode
            );
        }

        return codeGen.join(
            valueCode,
            target.value.generateCode(options),
            codeGen.bytes(this.isLocal ? codeGen.vxcTokens.VAR : codeGen.vxcTokens.SET)
        );
    }

    findDescendantsOfTypes(nodeTypes, notParentOf = []) {
        return [
            ...super.findDescendantsOfTypes(nodeTypes, notParentOf),
            ...this.targetInstance.findDescendantsOfTypes(nodeTypes, notParentOf)
        ];
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
            UnaryBitwiseNotOperatorExpressionNode,
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

        if (this.want(tokens, [new ast.TokenQuery(tokeniser.AssignmentOperatorToken)])) {
            return ExpressionAssignmentNode.create(tokens, namespace, instance, assigningToLocalVariable);
        }

        if (assigningToLocalVariable) {
            return ExpressionAssignmentNode.create(tokens, namespace, instance, true);
        }

        return instance;
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

export class UnaryBitwiseNotOperatorExpressionNode extends UnaryPrefixOperatorExpressionNode {
    static MATCH_QUERIES = [new ast.TokenQuery(tokeniser.OperatorToken, "~")];

    static OPERATOR_CODE = codeGen.join(
        codeGen.number(1),
        codeGen.systemCall("~x")
    );

    estimateTruthiness() {
        return null;
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
    static OPERATOR_CODE = ALL_BINARY_OPERATOR_CODE;
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

    static CHILD_EXPRESSION_NODE_CLASS = MultiplicationDivisionOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class BitwiseShiftOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "<<"),
        new ast.TokenQuery(tokeniser.OperatorToken, ">>"),
        new ast.TokenQuery(tokeniser.OperatorToken, ">>>")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = AdditionSubtractionOperatorExpressionNode;

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
        new ast.TokenQuery(tokeniser.OperatorToken, "!=="),
        new ast.TokenQuery(tokeniser.OperatorToken, "!="),
        new ast.TokenQuery(tokeniser.OperatorToken, "==="),
        new ast.TokenQuery(tokeniser.OperatorToken, "==")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = BitwiseShiftOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class InstanceOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "is"),
        new ast.TokenQuery(tokeniser.OperatorToken, "inherits")
    ];

    static OPERATOR_CODE = ALL_BINARY_OPERATOR_CODE;
    static CHILD_EXPRESSION_NODE_CLASS = EqualityOperatorExpressionNode;

    static TYPE_NAMES_TO_BYTES = {
        "Boolean": "t",
        "Byte": "b",
        "Function": "@",
        "ClosureFunction": "C",
        "Number": "%",
        "Buffer": "B",
        "String": "\"",
        "Object": "O",
        "List": "L",
        "WeakReference": "W"
    };

    operatorToken = null;
    typeByteUsed = null;

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [this.CHILD_EXPRESSION_NODE_CLASS], namespace);

        instance.operatorToken = this.maybeEat(tokens, this.OPERATOR_TOKEN_QUERIES);

        if (!instance.operatorToken) {
            return instance.children[0];
        }

        var typeNameToken = this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.TypeNameToken)]);

        if (instance.operatorToken.value == "is" && typeNameToken) {
            instance.typeByteUsed = this.TYPE_NAMES_TO_BYTES[typeNameToken.value];
        } else {
            instance.expectChildByMatching(tokens, [this.CHILD_EXPRESSION_NODE_CLASS], namespace);
        }

        return instance;
    }

    generateCode(options) {
        if (this.typeByteUsed != null) {
            return codeGen.join(
                this.children[0].generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.BYTE, codeGen.byte(this.typeByteUsed)),
                codeGen.number(2),
                codeGen.systemCall("Tt")
            );
        }

        return codeGen.join(
            this.children[0].generateCode(options),
            this.children[1].generateCode(options),
            ALL_BINARY_OPERATOR_CODE[this.operatorToken.value]
        );
    }
}

export class BitwiseAndOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "&")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = InstanceOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class BitwiseXorOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "^")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = BitwiseAndOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class BitwiseOrOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "|")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = BitwiseXorOperatorExpressionNode;

    estimateTruthiness() {
        return null;
    }
}

export class LogicalEagerAndOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "&&&")
    ];

    static CHILD_EXPRESSION_NODE_CLASS = BitwiseOrOperatorExpressionNode;

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

export class TernaryOperatorExpressionNode extends ExpressionNode {
    static CHILD_EXPRESSION_NODE_CLASS = NullishCoalescingOperatorExpressionNode;

    skipTrueSymbol = null;
    skipFalseSymbol = null;
    conditionTruthiness = null;

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [this.CHILD_EXPRESSION_NODE_CLASS], namespace);
        
        if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.OperatorToken, "?")])) {
            return instance.children[0];
        }

        instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

        if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.PropertyDefinerToken)])) {
            var token = tokens[0];

            var tokenHumanReadableName = token ? token.constructor.HUMAN_READABLE_NAME : "nothing";

            throw new sources.SourceError(`Expected ternary result delimeter (:) but got ${tokenHumanReadableName} instead`, token?.sourceContainer, token?.location);
        }

        instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

        instance.skipTrueSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("ternary_true"));
        instance.skipFalseSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("ternary_false"));

        return instance;
    }

    checkSymbolUsage(scope) {
        this.scope = scope;

        this.children[0].checkSymbolUsage(scope);

        if (this.conditionTruthiness == null) {
            this.conditionTruthiness = this.children[0].estimateTruthiness();
        }

        if (this.conditionTruthiness == true) {
            this.children[1].checkSymbolUsage(scope);

            return;
        }

        if (this.conditionTruthiness == false) {
            this.children[2].checkSymbolUsage(scope);

            return;
        }
    }

    estimateTruthiness() {
        if (this.conditionTruthiness == true) {
            return this.children[1].estimateTruthiness();
        }

        if (this.conditionTruthiness == false) {
            return this.children[2].estimateTruthiness();
        }

        return null;
    }

    generateCode(options) {
        if (options.removeDeadCode) {
            if (this.conditionTruthiness != null) {
                return this.conditionTruthiness ? this.children[1].generateCode(options) : this.children[2].generateCode(options);
            }
        }

        var conditionCode = this.children[0].generateCode(options);
        var isTrueCode = this.children[1].generateCode(options);
        var isFalseCode = this.children[2].generateCode(options);

        var skipFalseCode = codeGen.join(
            this.skipFalseSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY)
        );

        var skipTrueCode = codeGen.join(
            this.skipTrueSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );

        var skipFalseDefinitionCode = codeGen.join(
            this.skipFalseSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(conditionCode.length + skipFalseCode.length + isFalseCode.length + skipTrueCode.length)
        );

        var skipTrueDefinitionCode = codeGen.join(
            this.skipTrueSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(skipFalseDefinitionCode.length + conditionCode.length + skipFalseCode.length + isFalseCode.length + skipTrueCode.length + isTrueCode.length)
        );

        return codeGen.join(
            skipTrueDefinitionCode,
            skipFalseDefinitionCode,
            conditionCode,
            skipFalseCode,
            isFalseCode,
            skipTrueCode,
            isTrueCode
        );
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