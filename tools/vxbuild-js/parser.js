import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";

export class TokenQuery {
    constructor(tokenType, targetValue = null, humanReadableName = null) {
        this.tokenType = tokenType;
        this.targetValue = targetValue;
        this._humanReadableName = humanReadableName;
    }

    get humanReadableName() {
        if (this._humanReadableName != null) {
            return this._humanReadableName;
        }

        if (this.targetValue == null) {
            return this.tokenType.HUMAN_READABLE_NAME;
        }

        return this.targetValue;
    }

    test(token) {
        return this.tokenType.matches(token, this.targetValue);
    }

    static want(token, queries) {
        for (var query of queries) {
            if (query.test(token)) {
                return true;
            }
        }

        return false;
    }

    static expect(token, queries) {
        var humanReadableNames = [];

        for (var query of queries) {
            if (query.test(token)) {
                return true;
            }

            humanReadableNames.push(query.humanReadableName);
        }

        if (humanReadableNames.length == 0) {
            throw new Error("At least one token type should be expected");
        }

        var tokenHumanReadableName = token ? token.constructor.HUMAN_READABLE_NAME : "nothing";

        throw new SyntaxError(`Expected ${humanReadableNames.join(" or ")} but got ${tokenHumanReadableName} instead`);
    }
}

export class AstNode {
    static HUMAN_READABLE_NAME = "AST node";
    static MATCH_QUERIES = [];

    constructor() {
        this.children = [];
    }

    static matches(tokens) {
        return TokenQuery.want(tokens[0], this.MATCH_QUERIES);
    }

    static matchChild(tokens, nodeTypes, namespace) {
        for (var nodeType of nodeTypes) {
            if (nodeType.matches(tokens)) {
                return nodeType.create(tokens, namespace);
            }
        }

        return null;
    }

    static want(tokens, queries = this.MATCH_QUERIES) {
        return TokenQuery.want(tokens[0], queries);
    }

    static eat(tokens, queries = this.MATCH_QUERIES) {
        if (queries != null) {
            TokenQuery.expect(tokens[0], queries);
        }

        return tokens.shift();
    }

    static maybeEat(tokens, queries = this.MATCH_QUERIES) {
        if (this.want(tokens, queries)) {
            return this.eat(tokens, null);
        }

        return null;
    }

    static create(tokens, namespace) {
        throw new Error("Not implemented on base class");
    }

    addChildByMatching(tokens, nodeTypes, namespace) {
        var match = this.constructor.matchChild(tokens, nodeTypes, namespace);

        if (match) {
            this.children.push(match);

            return match;
        }

        return null;
    }

    expectChildByMatching(tokens, nodeTypes, namespace) {
        var addedChild = this.addChildByMatching(tokens, nodeTypes, namespace);

        if (!addedChild) {
            var tokenHumanReadableName = tokens[0] ? tokens[0].constructor.HUMAN_READABLE_NAME : "nothing";

            throw new SyntaxError(`Expected ${nodeTypes.map((node) => node.HUMAN_READABLE_NAME).join(" or ")} but got ${tokenHumanReadableName}`);
        }
    }
}

export class ModuleNode extends AstNode {
    static HUMAN_READABLE_NAME = "module";

    static matches(tokens) {
        return true;
    }

    static create(tokens, namespace) {
        var instance = new this();

        while (true) {
            if (tokens.length == 0) {
                break;
            }

            if (this.maybeEat(tokens, [new TokenQuery(tokeniser.StatementDelimeterToken)])) {
                continue;
            }

            instance.expectChildByMatching(tokens, [StatementNode], namespace);
        }

        return instance;
    }
}

export class StatementNode extends AstNode {
    static HUMAN_READABLE_NAME = "statement";

    static matches(tokens) {
        return true;
    }

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [FunctionNode, ExpressionNode], namespace);

        return instance;
    }
}

export class StatementBlockNode extends AstNode {
    static HUMAN_READABLE_NAME = "statement block";

    static matches(tokens) {
        return true;
    }

    static create(tokens, namespace) {
        var instance = new this();

        if (this.maybeEat(tokens, [new TokenQuery(tokeniser.BracketToken, "{")])) {
            while (true) {
                if (this.maybeEat(tokens, [new TokenQuery(tokeniser.BracketToken, "}")])) {
                    break;
                }

                if (this.maybeEat(tokens, [new TokenQuery(tokeniser.StatementDelimeterToken)])) {
                    continue;
                }

                if (instance.addChildByMatching(tokens, [StatementNode], namespace)) {
                    continue;
                }

                throw new SyntaxError("Expected statement or }");
            }
        } else {
            instance.expectChildByMatching(tokens, [StatementNode], namespace);
        }

        return instance;
    }
}

export class FunctionParametersNode extends AstNode {
    static HUMAN_READABLE_NAME = "parameter list";

    static MATCH_QUERIES = [
        new TokenQuery(tokeniser.BracketToken, "(")
    ];

    parameters = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var addedFirstParameter = false;

        while (true) {
            if (this.maybeEat(tokens, [new TokenQuery(tokeniser.BracketToken, ")")])) {
                break;
            }

            if (addedFirstParameter) {
                this.eat(tokens, [new TokenQuery(tokeniser.DelimeterToken)]);
            }

            instance.parameters.push(
                new namespaces.Symbol(namespace, this.eat(tokens, [new TokenQuery(tokeniser.IdentifierToken)]).value)
            );

            addedFirstParameter = true;
        }

        return instance;
    }
}

export class FunctionNode extends AstNode {
    static HUMAN_READABLE_NAME = "function";

    static MATCH_QUERIES = [
        new TokenQuery(tokeniser.KeywordToken, "function")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens); // `function` keyword
        this.eat(tokens, [new TokenQuery(tokeniser.IdentifierToken)]);

        instance.expectChildByMatching(tokens, [FunctionParametersNode], namespace);
        instance.expectChildByMatching(tokens, [StatementBlockNode], namespace);

        return instance;
    }
}

export class ThingNode extends AstNode {
    static HUMAN_READABLE_NAME = "thing expression";

    static MATCH_QUERIES = [
        new TokenQuery(tokeniser.IdentifierToken),
        new TokenQuery(tokeniser.StringToken)
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
}

export class FunctionArgumentsNode extends AstNode {
    static HUMAN_READABLE_NAME = "argument list";

    static MATCH_QUERIES = [
        new TokenQuery(tokeniser.BracketToken, "(")
    ];

    arguments = [];

    static create(tokens, namespace) {
        var instance = new this();

        this.eat(tokens);

        var addedFirstArgument = false;

        while (true) {
            if (this.maybeEat(tokens, [new TokenQuery(tokeniser.BracketToken, ")")])) {
                break;
            }

            if (addedFirstArgument) {
                this.eat(tokens, [new TokenQuery(tokeniser.DelimeterToken)]);
            }

            instance.arguments.push(
                instance.expectChildByMatching(tokens, [ExpressionNode], namespace)
            );

            addedFirstArgument = true;
        }

        return instance;
    }
}

export class FunctionCallNode extends AstNode {
    static HUMAN_READABLE_NAME = "function call";

    static MATCH_QUERIES = [
        new TokenQuery(tokeniser.BracketToken, "(")
    ];

    static create(tokens, namespace) {
        var instance = new this();

        instance.expectChildByMatching(tokens, [FunctionArgumentsNode], namespace);

        return instance;
    }
}

export class ExpressionNode extends AstNode {
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
}

export function parse(tokens) {
    return ModuleNode.create(tokens, new namespaces.Namespace());
}