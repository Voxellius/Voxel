import * as sources from "./sources.js";
import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";
import * as statements from "./statements.js";

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
        scope.addCoreNamespaceSymbol(this.getThisSymbol);

        super.checkSymbolUsage(scope);
    }

    generateCode() {
        return codeGen.join(
            codeGen.number(0),
            this.getThisSymbol.generateCode(),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
        );
    }
};

export class ThingNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "thing expression";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.AtomToken),
        new ast.TokenQuery(tokeniser.IdentifierToken),
        new ast.TokenQuery(tokeniser.StringToken),
        new ast.TokenQuery(tokeniser.NumberToken)
    ];

    value = null;

    static create(tokens, namespace) {
        var instance = new this();

        var token = this.eat(tokens);

        if (token instanceof tokeniser.AtomToken) {
            instance.value = {
                "true": true,
                "false": false,
                "null": null
            }[token.value] ?? null;
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

    checkSymbolUsage(scope) {
        scope.addSymbol(this.value);

        if (this.value instanceof namespaces.ForeignSymbolReference) {
            var usage = new namespaces.ForeignSymbolUsage(this.value.symbolName, this.value.subjectNamespaceIdentifier);

            usage.everRead = true;

            scope.foreignSymbolUses.push(usage);
        }

        super.checkSymbolUsage(scope);
    }

    generateCode() {
        if (
            this.value instanceof namespaces.Symbol ||
            this.value instanceof namespaces.ForeignSymbolReference
        ) {
            return codeGen.join(
                this.value.generateCode(),
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

    propertyNames = [];

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

            instance.propertyNames.push(this.eat(tokens, [
                new ast.TokenQuery(tokeniser.IdentifierToken),
                new ast.TokenQuery(tokeniser.StringToken)
            ]));
            
            this.eat(tokens, [new ast.TokenQuery(tokeniser.PropertyDefinerToken)]);
            
            instance.expectChildByMatching(tokens, [ExpressionNode], namespace);

            addedFirstItem = true;
        }

        return instance;
    }

    // TODO: Allow symbol usage to be tracked for object keys

    generateCode() {
        return codeGen.join(
            codeGen.number(0),
            codeGen.systemCall("O"),
            ...this.children.map((child, i) => codeGen.join(
                codeGen.bytes(codeGen.vxcTokens.DUPE),
                child.generateCode(),
                codeGen.bytes(codeGen.vxcTokens.SWAP),
                codeGen.string(this.propertyNames[i].value),
                codeGen.number(3),
                codeGen.systemCall("Os"),
                codeGen.bytes(codeGen.vxcTokens.POP, codeGen.vxcTokens.POP)
            ))
        );
    }
}

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

    generateCode() {
        return codeGen.join(
            ...this.children.map((child) => child.generateCode()),
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

    generateCode() {
        return codeGen.join(
            codeGen.number(this.parameters.length),
            codeGen.systemCall("P"),
            ...this.parameters.reverse().map((symbol) => codeGen.join(
                symbol.generateCode(),
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
    skipSymbol = null;
    capturedSymbols = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var identifier = this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

        instance.identifierSymbol = new namespaces.Symbol(namespace, identifier != null ? identifier.value : namespaces.generateSymbolName("anonfn"));
        instance.skipSymbol = new namespaces.Symbol(namespace, "#fn");

        instance.expectChildByMatching(tokens, [FunctionParametersNode], namespace);

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "captures")])) {
            while (true) {
                var capturedIdentifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

                instance.capturedSymbols.push(new namespaces.Symbol(namespace, capturedIdentifier.value));

                if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)])) {
                    break;
                }
            }
        }

        instance.expectChildByMatching(tokens, [statements.StatementBlockNode], namespace);

        return instance;
    }

    checkSymbolUsage(scope) {
        scope.addSymbol(this.identifierSymbol, false, true);

        for (var capturedSymbol of this.capturedSymbols) {
            scope.addSymbol(capturedSymbol);
        }

        super.checkSymbolUsage(scope, true);
    }

    generateCode() {
        var symbolCode = this.identifierSymbol.generateCode();

        var bodyCode = codeGen.join(
            this.children[0].generateCode(), // Function parameters
            this.children[1].generateCode(), // Function statement block
            codeGen.bytes(codeGen.vxcTokens.NULL, codeGen.vxcTokens.RETURN)
        );

        var skipJumpCode = codeGen.join(
            this.skipSymbol.generateCode(),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );

        var storageCode = codeGen.join(
            symbolCode,
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(skipJumpCode.length)
        );

        var skipDefinitionCode = codeGen.join(
            this.skipSymbol.generateCode(),
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
                    symbol.generateCode(),
                    codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.SWAP),
                    symbol.generateCode(),
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
}

export class FunctionArgumentsNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "argument list";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.BracketToken, "(")
    ];

    arguments = [];

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

            instance.arguments.push(
                instance.expectChildByMatching(tokens, [ExpressionNode], namespace)
            );

            addedFirstArgument = true;
        }

        return instance;
    }

    generateCode() {
        return codeGen.join(
            ...this.children.map((child) => child.generateCode()),
            codeGen.number(this.children.length)
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

    checkSymbolUsage(scope) {
        scope.addCoreNamespaceSymbol(this.pushThisSymbol);
        scope.addCoreNamespaceSymbol(this.popThisSymbol);

        super.checkSymbolUsage(scope);
    }

    generateCode(expressionCode, calledAsMethod) {
        if (calledAsMethod) {
            return codeGen.join(
                this.children[0].generateCode(),
                expressionCode,
                codeGen.number(0),
                this.pushThisSymbol.generateCode(),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP),
                codeGen.bytes(codeGen.vxcTokens.CALL),
                codeGen.number(0),
                this.popThisSymbol.generateCode(),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL, codeGen.vxcTokens.POP)
            );
        }

        return codeGen.join(
            this.children[0].generateCode(),
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

    generateCode() {
        return codeGen.join(
            this.children[0].generateCode(),
            codeGen.number(2),
            codeGen.systemCall("Tg")
        );
    }

    generateSetterCode(targetCode, valueCode) {
        return codeGen.join(
            valueCode,
            targetCode,
            this.children[0].generateCode(),
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
        instance.property = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value;

        return instance;
    }

    checkSymbolUsage(scope) {
        scope.addCoreNamespaceSymbol(this.getPropertySymbol);
        scope.addCoreNamespaceSymbol(this.setPropertySymbol);

        super.checkSymbolUsage(scope);
    }

    generateCode() {
        return codeGen.join(
            codeGen.string(this.property),
            codeGen.number(2),
            this.getPropertySymbol.generateCode(),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL)
        );
    }

    generateSetterCode(targetCode, valueCode) {
        return codeGen.join(
            targetCode,
            codeGen.string(this.property),
            valueCode,
            codeGen.number(3),
            this.setPropertySymbol.generateCode(),
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
        ...ThingNode.MATCH_QUERIES,
        ...ObjectNode.MATCH_QUERIES,
        ...ListNode.MATCH_QUERIES,
        ...FunctionNode.MATCH_QUERIES,
        new ast.TokenQuery(tokeniser.OperatorToken, "-"),
        new ast.TokenQuery(tokeniser.OperatorToken, "!")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [NullishCoalescingOperatorExpressionNode], namespace);

        return instance;
    }

    generateCode() {
        return codeGen.join(...this.children.map((child) => child.generateCode()));
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
        var target = this.targetInstance.children[0];

        if (target.value instanceof namespaces.Symbol) {
            var usage = scope.getSymbolById(target.value.id, true);

            usage.everDefined = true;
        }

        super.checkSymbolUsage(scope);
    }

    generateCode() {
        var valueCode = this.children.length > 0 ? this.children[0].generateCode() : codeGen.bytes(codeGen.vxcTokens.NULL);
        var target = this.targetInstance.children.pop();

        if (this.targetInstance.children.length > 0) {
            if (target instanceof IndexAccessorNode || target instanceof PropertyAccessorNode) {
                return target.generateSetterCode(
                    this.targetInstance.generateCode(),
                    valueCode
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

        return codeGen.join(
            valueCode,
            target.value.generateCode(),
            codeGen.bytes(this.isLocal ? codeGen.vxcTokens.VAR : codeGen.vxcTokens.SET)
        );
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

        if (instance.addChildByMatching(tokens, [UnaryNegativeOperatorExpressionNode, UnaryNotOperatorExpressionNode], namespace)) {
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

    generateCode() {
        var currentCode = codeGen.bytes();
        var lastNodeWasPropertyAccessor = false;

        for (var child of this.children) {
            if (child instanceof FunctionCallNode) {
                // Function calls need to push the arguments passed to the function first before the function thing is pushed on
                currentCode = child.generateCode(currentCode, lastNodeWasPropertyAccessor);

                continue;
            }

            currentCode = codeGen.join(currentCode, child.generateCode());
            lastNodeWasPropertyAccessor = child instanceof PropertyAccessorNode;
        }

        return currentCode;
    }
}

export class ExpressionThingNode extends ExpressionLeafNode {
    static MATCH_QUERIES = [
        ...ThisNode.MATCH_QUERIES,
        ...ThingNode.MATCH_QUERIES,
        ...ObjectNode.MATCH_QUERIES,
        ...ListNode.MATCH_QUERIES,
        ...FunctionNode.MATCH_QUERIES
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [ThisNode, ThingNode, ObjectNode, ListNode, FunctionNode], namespace);

        this.maybeAddAccessors(instance, tokens, namespace);

        return instance;
    }
}

export class UnaryOperatorExpressionNode extends ExpressionNode {
    static MATCH_QUERIES = [];
    static OPERATOR_CODE = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.expectChildByMatching(tokens, [ExpressionLeafNode], namespace);

        return instance;
    }

    generateCode() {
        return codeGen.join(this.children[0].generateCode(), this.constructor.OPERATOR_CODE);
    }
}

export class UnaryNegativeOperatorExpressionNode extends UnaryOperatorExpressionNode {
    static MATCH_QUERIES = [new ast.TokenQuery(tokeniser.OperatorToken, "-")];

    static OPERATOR_CODE = codeGen.join(
        codeGen.number(1),
        codeGen.systemCall("-x")
    );
}

export class UnaryNotOperatorExpressionNode extends UnaryOperatorExpressionNode {
    static MATCH_QUERIES = [new ast.TokenQuery(tokeniser.OperatorToken, "!")];

    static OPERATOR_CODE = codeGen.bytes(codeGen.vxcTokens.NOT);
}

export class BinaryOperatorExpressionNode extends ExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [];
    static OPERATOR_CODE = {};
    static CHILD_EXPRESSION_NODE_CLASS = ExpressionLeafNode;

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

        return instance;
    }

    generateCode() {
        var currentCode = this.children[0].generateCode();

        for (var i = 1; i < this.children.length; i++) {
            var child = this.children[i];
            var operator = this.operatorTokens[i - 1];

            currentCode = codeGen.join(
                currentCode,
                child.generateCode(),
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

    static CHILD_EXPRESSION_NODE_CLASS = ExpressionLeafNode;
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
}

export class LogicalEagerAndOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "&&&")
    ];

    static OPERATOR_CODE = {
        "&&&": codeGen.bytes(codeGen.vxcTokens.AND)
    };

    static CHILD_EXPRESSION_NODE_CLASS = EqualityOperatorExpressionNode;
}

export class LogicalEagerOrOperatorExpressionNode extends BinaryOperatorExpressionNode {
    static OPERATOR_TOKEN_QUERIES = [
        new ast.TokenQuery(tokeniser.OperatorToken, "|||")
    ];

    static OPERATOR_CODE = {
        "|||": codeGen.bytes(codeGen.vxcTokens.OR)
    };

    static CHILD_EXPRESSION_NODE_CLASS = LogicalEagerAndOperatorExpressionNode;
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

    generateCode() {
        if (this.children.length == 1) {
            return this.children[0].generateCode();
        }

        var allChildrenCode = this.children.map((child) => child.generateCode());
        var skipSymbolCode = this.skipSymbol.generateCode();
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

    generateCode() {
        if (this.children.length == 1) {
            return this.children[0].generateCode();
        }

        var allChildrenCode = this.children.map((child) => child.generateCode());
        var skipSymbolCode = this.skipSymbol.generateCode();
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

    generateCode() {
        if (this.children.length == 1) {
            return this.children[0].generateCode();
        }

        var allChildrenCode = this.children.map((child) => child.generateCode());
        var skipSymbolCode = this.skipSymbol.generateCode();
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

    generateCode() {
        return codeGen.join(
            this.children[0].generateCode(),
            this.value.generateCode()
        );
    }
}