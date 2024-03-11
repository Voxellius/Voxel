import * as sources from "./sources.js";
import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";

export class ThingNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "thing expression";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.IdentifierToken),
        new ast.TokenQuery(tokeniser.StringToken),
        new ast.TokenQuery(tokeniser.NumberToken)
    ];

    value = null;

    static create(tokens, namespace) {
        var instance = new this();

        var token = this.eat(tokens);

        if (token instanceof tokeniser.IdentifierToken) {
            instance.value = new namespaces.Symbol(namespace, token.value);
        } else if (token instanceof tokeniser.StringToken || token instanceof tokeniser.NumberToken) {
            instance.value = token.value;
        } else {
            throw new Error("Not implemented");
        }

        return instance;
    }

    generateCode() {
        if (this.value instanceof namespaces.Symbol) {
            return codeGen.join(
                this.value.generateCode(),
                codeGen.bytes(codeGen.vxcTokens.GET)
            );
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

        instance.expectChildByMatching(tokens, [FunctionArgumentsNode], namespace);

        return instance;
    }

    generateCode(expressionCode) {
        return codeGen.join(
            this.children[0].generateCode(),
            expressionCode,
            codeGen.bytes(codeGen.vxcTokens.CALL)
        );
    }
}

export class ExpressionNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "expression";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "syscall"),
        new ast.TokenQuery(tokeniser.KeywordToken, "var"),
        new ast.TokenQuery(tokeniser.BracketToken, "("),
        ...ThingNode.MATCH_QUERIES,
        new ast.TokenQuery(tokeniser.OperatorToken, "-")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [AdditionSubtractionOperatorExpressionNode], namespace);

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

    generateCode() {
        var currentCode = codeGen.bytes();
        var target = this.targetInstance.children.pop();

        if (this.targetInstance.children.length > 0) {
            currentCode = currentCode.join(this.targetInstance.generateCode());

            console.log("Accessor parent's code:", currentCode);

            // TODO: Implement setting object and list items in bytecode
            throw new Error("Not implemented yet");
        }

        if (!(target instanceof ThingNode)) {
            throw new Error("Trap: setter target is not a `ThingNode`");
        }

        if (!(target.value instanceof namespaces.Symbol)) {
            // TODO: Specify token
            throw new sources.SourceError("Cannot set a value literal (expected a variable name)");
        }

        return codeGen.join(
            currentCode,
            this.children.length > 0 ? this.children[0].generateCode() : codeGen.bytes(codeGen.vxcTokens.NULL),
            target.value.generateCode(),
            codeGen.bytes(this.isLocal ? codeGen.vxcTokens.VAR : codeGen.vxcTokens.SET)
        );
    }
}

export class ExpressionLeafNode extends ExpressionNode {
    static maybeAddAccessors(instance, tokens, namespace) {
        while (true) {
            if (instance.addChildByMatching(tokens, [FunctionCallNode], namespace)) {
                continue;
            }

            break;
        }
    }

    static create(tokens, namespace) {
        var instance = new this();
        var assigningToLocalVariable = false;

        if (instance.addChildByMatching(tokens, [UnaryNegativeOperatorExpressionNode], namespace)) {
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

        for (var child of this.children) {
            if (child instanceof FunctionCallNode) {
                // Function calls need to push the arguments passed to the function first before the function thing is pushed on
                currentCode = child.generateCode(currentCode);

                continue;
            }

            currentCode = codeGen.join(currentCode, child.generateCode());
        }

        return currentCode;
    }
}

export class ExpressionThingNode extends ExpressionLeafNode {
    static MATCH_QUERIES = [
        ...ThingNode.MATCH_QUERIES
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [ThingNode], namespace);

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
        new ast.TokenQuery(tokeniser.OperatorToken, "/")
    ];

    static OPERATOR_CODE = {
        "*": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("*")
        ),
        "/": codeGen.join(
            codeGen.number(2),
            codeGen.systemCall("/")
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
            codeGen.systemCall("/")
        )
    };

    static CHILD_EXPRESSION_NODE_CLASS = MultiplicationDivisionOperatorExpressionNode;
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