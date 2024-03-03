import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";

export class ThingNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "thing expression";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.IdentifierToken),
        new ast.TokenQuery(tokeniser.StringToken)
    ];

    value = null;

    static create(tokens, namespace) {
        var instance = new this();

        var token = this.eat(tokens);

        if (token instanceof tokeniser.IdentifierToken) {
            instance.value = new namespaces.Symbol(namespace, token.value);
        } else if (token instanceof tokeniser.StringToken) {
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
        ...ThingNode.MATCH_QUERIES
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [ThingNode], namespace);

        while (true) {
            if (instance.addChildByMatching(tokens, [FunctionCallNode], namespace)) {
                continue;
            }

            break;
        }

        return instance;
    }

    generateCode() {
        var children = [...this.children];
        var lastChild = children[children.length - 1];

        if (lastChild instanceof FunctionCallNode) {
            children.pop();

            return lastChild.generateCode(codeGen.join(...children.map((child) => child.generateCode())));
        }

        return codeGen.join(...children.map((child) => child.generateCode()));
    }
}