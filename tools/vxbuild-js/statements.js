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

        instance.expectChildByMatching(tokens, [ImportStatementNode, IfStatementNode, WhileLoopNode, ForLoopNode, ReturnStatementNode, expressions.ExpressionNode], namespace);

        return instance;
    }

    generateCode(options) {
        if (this.children[0] instanceof expressions.ExpressionNode) {
            return codeGen.join(this.children[0].generateCode(options), codeGen.bytes(codeGen.vxcTokens.POP));
        }

        return this.children[0].generateCode(options);
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

    generateCode(options) {
        return codeGen.join(...this.children.map((child) => child.generateCode(options)));
    }
}

export class ImportStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "import statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "import")
    ];

    identifierSymbol = null;
    skipSymbol = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var location = this.eat(tokens, [new ast.TokenQuery(tokeniser.StringToken)]).value;

        this.eat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "as")]);

        var identifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value;

        namespace.import(location, identifier);

        return instance;
    }

    generateCode(options) {
        return codeGen.bytes();
    }
}

export class ReturnStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`return` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "return")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.addChildByMatching(tokens, [expressions.ExpressionNode], namespace);

        return instance;
    }

    generateCode(options) {
        return codeGen.join(
            this.children[0] ? this.children[0].generateCode(options) : codeGen.bytes(codeGen.vxcTokens.NULL),
            codeGen.bytes(codeGen.vxcTokens.RETURN)
        );
    }
}

export class IfStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`if` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "if")
    ];

    skipTrueSymbol = null;
    skipFalseSymbol = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);
        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")]);

        instance.expectChildByMatching(tokens, [expressions.ExpressionNode], namespace);

        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")]);

        instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

        instance.skipTrueSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("if_true"));

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "else")])) {
            instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

            instance.skipFalseSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("if_false"));
        }

        return instance;
    }

    generateCode(options) {
        if (options.removeDeadCode) {
            var conditionTruthiness = this.children[0].estimateTruthiness();

            if (conditionTruthiness != null) {
                return codeGen.join(
                    this.children[0].generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.POP),
                    conditionTruthiness ? (
                        this.children[1].generateCode(options)
                    ) : (
                        this.skipFalseSymbol ?
                        this.children[2].generateCode(options) :
                        codeGen.bytes()
                    )
                );
            }
        }

        var notConditionCode = codeGen.join(
            this.children[0].generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.NOT)
        );

        var isTrueCode = codeGen.join(
            this.children[1].generateCode(options)
        );

        var isFalseCode = this.skipFalseSymbol ? codeGen.join(
            this.children[2].generateCode(options)
        ) : codeGen.bytes();

        var skipFalseCode = this.skipFalseSymbol ? codeGen.join(
            this.skipFalseSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        ) : codeGen.bytes(); // Not necessary if we do not need to skip true statement

        var skipTrueCode = codeGen.join(
            this.skipTrueSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY)
        );

        var skipFalseDefinitionCode = this.skipFalseSymbol ? codeGen.join(
            this.skipFalseSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(notConditionCode.length + skipTrueCode.length + isTrueCode.length + skipFalseCode.length + isFalseCode.length)
        ) : codeGen.bytes();

        var skipTrueDefinitionCode = codeGen.join(
            this.skipTrueSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(skipFalseDefinitionCode.length + notConditionCode.length + skipTrueCode.length + isTrueCode.length + skipFalseCode.length)
        );

        return codeGen.join(
            skipTrueDefinitionCode,
            skipFalseDefinitionCode,
            notConditionCode,
            skipTrueCode,
            isTrueCode,
            skipFalseCode,
            isFalseCode
        );
    }
}

export class WhileLoopNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`while` loop";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "while")
    ];

    skipLoopSymbol = null;
    repeatLoopSymbol = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);
        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")]);

        instance.expectChildByMatching(tokens, [expressions.ExpressionNode], namespace);

        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")]);

        instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

        instance.skipLoopSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("while_skip"));
        instance.repeatLoopSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("while_loop"));

        return instance;
    }

    generateCode(options) {
        var loopCondition = this.children[0];
        var loopStatementBlock = this.children[1];

        if (options.removeDeadCode) {
            var conditionTruthiness = loopCondition.estimateTruthiness();

            if (conditionTruthiness == false) {
                return codeGen.join(
                    loopCondition.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.POP)
                );
            }
        }

        var notConditionCode = codeGen.join(
            loopCondition.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.NOT)
        );

        var loopCode = codeGen.join(
            loopStatementBlock.generateCode(options)
        );

        var skipLoopCode = codeGen.join(
            this.skipLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY)
        );

        var repeatLoopCode = codeGen.join(
            this.repeatLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );

        var repeatLoopDefinitionCode = codeGen.join(
            this.repeatLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_HERE)
        );

        var skipLoopDefinitionCode = codeGen.join(
            this.skipLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(repeatLoopDefinitionCode.length + notConditionCode.length + skipLoopCode.length + loopCode.length + repeatLoopCode.length)
        );

        return codeGen.join(
            skipLoopDefinitionCode,
            repeatLoopDefinitionCode,
            notConditionCode,
            skipLoopCode,
            loopCode,
            repeatLoopCode
        );
    }
}

export class ForLoopNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`for` loop";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "for")
    ];

    skipLoopSymbol = null;
    repeatLoopSymbol = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);
        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")]);

        if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.StatementDelimeterToken)])) {
            instance.expectChildByMatching(tokens, [StatementNode], namespace);
            this.eat(tokens, [new ast.TokenQuery(tokeniser.StatementDelimeterToken)]);
        } else {
            instance.children.push(expressions.ThingNode.createByValue(null));
        }

        if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.StatementDelimeterToken)])) {
            instance.expectChildByMatching(tokens, [expressions.ExpressionNode], namespace);
            this.eat(tokens, [new ast.TokenQuery(tokeniser.StatementDelimeterToken)]);
        } else {
            instance.children.push(expressions.ThingNode.createByValue(true));
        }

        if (!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")])) {
            instance.expectChildByMatching(tokens, [StatementNode], namespace);
            this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")]);
        } else {
            instance.children.push(expressions.ThingNode.createByValue(null));
        }

        instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

        instance.skipLoopSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("while_skip"));
        instance.repeatLoopSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("while_loop"));

        return instance;
    }

    generateCode(options) {
        var startStatement = this.children[0];
        var stopCondition = this.children[1];
        var stepStatement = this.children[2];
        var loopStatementBlock = this.children[3];

        if (options.removeDeadCode) {
            var conditionTruthiness = stopCondition.estimateTruthiness();

            if (conditionTruthiness == false) {
                return codeGen.join(
                    startStatement.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.POP),
                    stopCondition.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.POP)
                );
            }
        }

        var startCode = startStatement.generateCode(options);

        var notConditionCode = codeGen.join(
            stopCondition.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.NOT)
        );

        var loopCode = codeGen.join(
            loopStatementBlock.generateCode(options)
        );

        var stepCode = stepStatement.generateCode(options);

        var skipLoopCode = codeGen.join(
            this.skipLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP_IF_TRUTHY)
        );

        var repeatLoopCode = codeGen.join(
            this.repeatLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );

        var repeatLoopDefinitionCode = codeGen.join(
            this.repeatLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_HERE)
        );

        var skipLoopDefinitionCode = codeGen.join(
            this.skipLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(repeatLoopDefinitionCode.length + notConditionCode.length + skipLoopCode.length + loopCode.length + stepCode.length + repeatLoopCode.length)
        );

        return codeGen.join(
            startCode,
            skipLoopDefinitionCode,
            repeatLoopDefinitionCode,
            notConditionCode,
            skipLoopCode,
            loopCode,
            stepCode,
            repeatLoopCode
        );
    }
}