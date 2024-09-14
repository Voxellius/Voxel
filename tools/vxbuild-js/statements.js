import * as path from "https://deno.land/std@0.220.1/path/mod.ts";

import * as common from "./common.js";
import * as sources from "./sources.js";
import * as tokeniser from "./tokeniser.js";
import * as namespaces from "./namespaces.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";
import * as dce from "./dce.js";
import * as expressions from "./expressions.js";

export class StatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "statement";

    static matches(tokens) {
        return true;
    }

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [
            ImportStatementNode,
            IfStatementNode,
            WhileLoopNode,
            ForLoopNode,
            TryStatementNode,
            ReturnStatementNode,
            ThrowStatementNode,
            EnumStatementNode,
            BreakStatementNode,
            ContinueStatementNode,
            DeleteStatementNode,
            expressions.ExpressionNode
        ], namespace);

        return instance;
    }

    generateCode(options) {
        if (this.children[0] instanceof expressions.ExpressionNode) {
            var generatedCode = this.children[0].generateCode(options);

            if (generatedCode.length == 1 && generatedCode[0] == codeGen.vxcTokens.NULL) {
                return codeGen.bytes();
            }

            return codeGen.join(generatedCode, codeGen.bytes(codeGen.vxcTokens.POP));
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

                throw new sources.SourceError("Expected statement or \`}\`", tokens[0]?.sourceContainer, tokens[0]?.location);
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

        var locationToken = this.eat(tokens, [new ast.TokenQuery(tokeniser.StringToken), new ast.TokenQuery(tokeniser.IdentifierToken)]);
        var isStandardLibraryImport = locationToken instanceof tokeniser.IdentifierToken;
        var location = locationToken.value;
        var identifier = location;

        if (isStandardLibraryImport) {
            location = path.resolve(common.STDLIB_DIR, location, `${location}.vxl`);

            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "as")])) {
                identifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value;
            }
        } else {
            this.eat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "as")]);

            identifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value;
        }

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
    conditionTruthiness = null;

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

        if (this.conditionTruthiness == false && this.skipFalseSymbol) {
            this.children[2].checkSymbolUsage(scope);

            return;
        }

        for (var child of this.children) {
            child.checkSymbolUsage(scope);
        }
    }

    generateCode(options) {
        if (options.removeDeadCode) {
            if (this.conditionTruthiness != null) {
                return codeGen.join(
                    dce.hasNoEffect(this, [this.children[0]]) ? codeGen.bytes() : codeGen.join(
                        this.children[0].generateCode(options),
                        codeGen.bytes(codeGen.vxcTokens.POP)
                    ),
                    this.conditionTruthiness ? (
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

        var isTrueCode = this.children[1].generateCode(options);

        var isFalseCode = this.skipFalseSymbol ? codeGen.join(
            this.children[2].generateCode(options)
        ) : codeGen.bytes();

        var skipFalseCode = this.skipFalseSymbol ? codeGen.join(
            codeGen.bytes(codeGen.vxcTokens.NULL, codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(isFalseCode.length + 1),
            codeGen.bytes(codeGen.vxcTokens.JUMP)

        ) : codeGen.bytes(); // Not necessary if we do not need to skip true statement

        var skipTrueCode = codeGen.join(
            codeGen.bytes(codeGen.vxcTokens.NULL, codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(isTrueCode.length + skipFalseCode.length + isFalseCode.length + 1),
            codeGen.bytes(codeGen.vxcTokens.JUMP_IF_TRUTHY)
        );

        return codeGen.join(
            // skipTrueDefinitionCode,
            // skipFalseDefinitionCode,
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
            instance.children.push(new ast.NullNode());
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
            instance.children.push(new ast.NullNode());
        }

        instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

        instance.skipLoopSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("for_skip"));
        instance.repeatLoopSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("for_loop"));
        instance.stepAndLoopSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("for_stepLoop"));

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

        var loopCode = loopStatementBlock.generateCode(options);
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

        var stepAndLoopDefinitionCode = codeGen.bytes();

        if (this.findDescendantsOfTypes([ContinueStatementNode], [expressions.FunctionNode, expressions.MethodNode])) {
            stepAndLoopDefinitionCode = codeGen.join(
                this.stepAndLoopSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
                codeGen.int32(repeatLoopDefinitionCode.length + notConditionCode.length + skipLoopCode.length + loopCode.length)
            );
        }

        var skipLoopDefinitionCode = codeGen.join(
            this.skipLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(stepAndLoopDefinitionCode.length + repeatLoopDefinitionCode.length + notConditionCode.length + skipLoopCode.length + loopCode.length + stepCode.length + repeatLoopCode.length)
        );

        return codeGen.join(
            startCode,
            skipLoopDefinitionCode,
            stepAndLoopDefinitionCode,
            repeatLoopDefinitionCode,
            notConditionCode,
            skipLoopCode,
            loopCode,
            stepCode,
            repeatLoopCode
        );
    }
}

export class ThrowStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`throw` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "throw")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.expectChildByMatching(tokens, [expressions.ExpressionNode], namespace);

        return instance;
    }

    generateCode(options) {
        return codeGen.join(this.children[0].generateCode(options), codeGen.bytes(codeGen.vxcTokens.THROW));
    }
}

export class TryStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`try` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "try")
    ];

    enterHandlerSymbol = null;
    skipHandlerSymbol = null;
    catchExceptionSymbol = null;

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

        if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.KeywordToken, "catch")])) {
            this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")]);

            var identifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

            instance.catchExceptionSymbol = new namespaces.Symbol(namespace, identifier.value);

            this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")]);

            instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);
        }

        instance.enterHandlerSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("try_handler"));
        instance.skipHandlerSymbol = new namespaces.Symbol(namespace, namespaces.generateSymbolName("try_skip"));

        return instance;
    }

    generateCode(options) {
        var trialCode = this.children[0].generateCode(options);

        var handlerCode = this.catchExceptionSymbol != null ? codeGen.join(
            this.catchExceptionSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.SET),
            codeGen.bytes(codeGen.vxcTokens.POP),
            this.children[1].generateCode(options),
        ) : codeGen.bytes(codeGen.vxcTokens.POP);

        var skipHandlerCode = codeGen.join(
            codeGen.bytes(codeGen.vxcTokens.CLEAR_HANDLER),
            this.skipHandlerSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );

        var setHandlerCode = codeGen.join(
            this.enterHandlerSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.SET_HANDLER)
        );

        var enterHandlerDefinitionCode = codeGen.join(
            this.enterHandlerSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(setHandlerCode.length + trialCode.length + skipHandlerCode.length)
        );

        var skipHandlerDefinitionCode = codeGen.join(
            this.skipHandlerSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.POS_REF_FORWARD),
            codeGen.int32(enterHandlerDefinitionCode.length + setHandlerCode.length + trialCode.length + skipHandlerCode.length + handlerCode.length)
        );

        return codeGen.join(
            skipHandlerDefinitionCode,
            enterHandlerDefinitionCode,
            setHandlerCode,
            trialCode,
            skipHandlerCode,
            handlerCode
        );
    }
}

export class EnumStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`enum` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "enum")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var enumIdentifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);
        var enumObject = {};
        var enumsToAutoFill = [];

        namespace.enums[enumIdentifier.value] = enumObject;

        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "{")]);

        var addedFirstEntry = false;

        while (true) {
            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "}")])) {
                break;
            }

            if (addedFirstEntry) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)]);
            }

            var entryIdentifier = this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]);

            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.AssignmentOperatorToken, "=")])) {
                var negate = !!this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.OperatorToken, "-")]);
                var entryValueToken = this.eat(tokens, [new ast.TokenQuery(tokeniser.NumberToken)]);
                var entryValue = entryValueToken.value * (negate ? -1 : 1);
    
                enumObject[entryIdentifier.value] = entryValue;

                namespaces.markEnumValueAsDefined(entryValue);
            } else {
                enumsToAutoFill.push(entryIdentifier);
            }

            addedFirstEntry = true;
        }

        for (var entryIdentifier of enumsToAutoFill) {
            enumObject[entryIdentifier.value] = namespaces.getNextAutoEnumValue();
        }

        return instance;
    }

    generateCode(options) {
        return codeGen.bytes();
    }
}

export class BreakStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`break` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "break")
    ];

    keywordToken = null;

    static create(tokens, namespace) {
        var instance = new this();

        instance.keywordToken = this.eat(tokens);

        return instance;
    }

    generateCode(options) {
        var parentLoop = this.findAncestorOfTypes(
            [WhileLoopNode, ForLoopNode],
            false,
            [expressions.FunctionNode, expressions.MethodNode]
        );

        if (!parentLoop) {
            throw new sources.SourceError("Cannot use `break` outside of `while` or `for` loop", this.keywordToken?.sourceContainer, this.keywordToken?.location);
        }

        return codeGen.join(
            parentLoop.skipLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );
    }
}

export class ContinueStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`continue` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "continue")
    ];

    keywordToken = null;

    static create(tokens, namespace) {
        var instance = new this();

        instance.keywordToken = this.eat(tokens);

        return instance;
    }

    generateCode(options) {
        var parentLoop = this.findAncestorOfTypes(
            [WhileLoopNode, ForLoopNode],
            false,
            [expressions.FunctionNode, expressions.MethodNode]
        );

        if (!parentLoop) {
            throw new sources.SourceError("Cannot use `continue` outside of `while` or `for` loop", this.keywordToken?.sourceContainer, this.keywordToken?.location);
        }

        if (parentLoop instanceof ForLoopNode) {
            return codeGen.join(
                parentLoop.stepAndLoopSymbol.generateCode(options),
                codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
            );
        }

        return codeGen.join(
            parentLoop.repeatLoopSymbol.generateCode(options),
            codeGen.bytes(codeGen.vxcTokens.GET, codeGen.vxcTokens.JUMP)
        );
    }
}

export class DeleteStatementNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "`delete` statement";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.KeywordToken, "delete")
    ];

    keywordToken = null;

    static create(tokens, namespace) {
        var instance = new this();

        instance.keywordToken = this.eat(tokens);

        instance.expectChildByMatching(tokens, [expressions.ExpressionThingNode], namespace);

        return instance;
    }

    generateCode(options) {
        var targetInstance = this.children[0];
        var target = targetInstance.children.at(-1);

        if (target instanceof expressions.FunctionCallNode) {
            throw new sources.SourceError("Expected an identifier (cannot delete the return value of a function)", this.keywordToken?.sourceContainer, this.keywordToken?.location);
        }

        if (target instanceof expressions.IndexAccessorNode || target instanceof expressions.PropertyAccessorNode) {
            var accessor = targetInstance.children.pop();

            return codeGen.join(
                targetInstance.generateCode(options),
                target instanceof expressions.PropertyAccessorNode ? accessor.propertySymbol.generateCode(options) : accessor.children[0].generateCode(options),
                codeGen.number(2),
                codeGen.systemCall("Or"),
                codeGen.bytes(codeGen.vxcTokens.POP)
            );
        }

        if (target instanceof expressions.ThingNode) {
            if (target.value instanceof namespaces.Symbol) {
                return codeGen.join(
                    target.value.generateCode(options),
                    codeGen.bytes(codeGen.vxcTokens.DELETE)
                );
            }
        }

        throw new sources.SourceError("Expected an identifier", this.keywordToken?.sourceContainer, this.keywordToken?.location);
    }
}