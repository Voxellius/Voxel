import * as tokeniser from "./tokeniser.js";
import * as namespaces from "./namespaces.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";
import * as expressions from "./expressions.js";

export class StatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "statement";

    static matches(tokens) {
        return true;
    }

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [FunctionNode, ReturnStatementNode, expressions.ExpressionNode], namespace);

        return instance;
    }

    generateCode() {
        if (
            this.children[0] instanceof FunctionNode ||
            this.children[0] instanceof expressions.ExpressionNode
        ) {
            return codeGen.join(this.children[0].generateCode(), codeGen.bytes(codeGen.vxcTokens.POP));
        }

        return this.children[0].generateCode();
    }
}

export class StatementBlockNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "statement block";

    static matches(tokens) {
        return true;
    }

    static create(tokens, namespace) {
        var instance = new this();

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "{")])) {
            while (true) {
                if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "}")])) {
                    break;
                }

                if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.StatementDelimeterToken)])) {
                    continue;
                }

                if (instance.addChildByMatching(tokens, [StatementNode], namespace)) {
                    continue;
                }

                throw new SyntaxError("Expected statement or \`}\`", tokens[0]?.location);
            }
        } else {
            instance.expectChildByMatching(tokens, [StatementNode], namespace);
        }

        return instance;
    }

    generateCode() {
        return codeGen.join(...this.children.map((child) => child.generateCode()));
    }
}

export class ReturnStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "return statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "return")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.addChildByMatching(tokens, [expressions.ExpressionNode], namespace);

        return instance;
    }

    generateCode() {
        return codeGen.join(
            this.children[0] ? this.children[0].generateCode() : codeGen.bytes(codeGen.vxcTokens.NULL),
            codeGen.bytes(codeGen.vxcTokens.RETURN)
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

    generateCode() {
        return codeGen.join(
            codeGen.number(this.parameters.length),
            codeGen.string("params"),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.CALL),
            ...this.parameters.reverse().map((symbol) => codeGen.join(
                symbol.generateCode(),
                codeGen.bytes(codeGen.vxcTokens.SET, codeGen.vxcTokens.POP)
            ))
        );
    }
}

export class FunctionNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "function";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "function")
    ];

    identifierSymbol = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens); // `function` keyword

        var identifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

        instance.identifierSymbol = new namespaces.Symbol(namespace, identifier.value);

        instance.expectChildByMatching(tokens, [FunctionParametersNode], namespace);
        instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

        return instance;
    }

    generateCode() {
        var symbolCode = this.identifierSymbol.generateCode();

        var bodyCode = codeGen.join(
            this.children[0].generateCode(), // Function parameters
            this.children[1].generateCode(), // Function statement block
            codeGen.bytes(codeGen.vxcTokens.NULL, codeGen.vxcTokens.RETURN)
        );

        var skipJumpCode = codeGen.join(
            codeGen.string("_fnskip"),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );

        var storageCode = codeGen.join(
            symbolCode,
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(skipJumpCode.length)
        );

        var skipDefinitionCode = codeGen.join(
            codeGen.string("_fnskip"), // TODO: Use better (perhaps generated) name
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(symbolCode.length + storageCode.length + skipJumpCode.length + bodyCode.length)
        );

        return codeGen.join(
            skipDefinitionCode,
            symbolCode,
            storageCode,
            skipJumpCode,
            bodyCode
        );
    }
}