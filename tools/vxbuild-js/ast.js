import * as sources from "./sources.js";

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

        return `\`${this.targetValue}\``;
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

        throw new sources.SourceError(
            `Expected ${humanReadableNames.join(" or ")} but got ${tokenHumanReadableName} instead`,
            token?.sourceContainer,
            token?.location
        );
    }
}

export class AstNode {
    static HUMAN_READABLE_NAME = "AST node";
    static MATCH_QUERIES = [];

    constructor() {
        this.children = [];
        this.scope = null;
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

            throw new sources.SourceError(
                `Expected ${nodeTypes.map((node) => node.HUMAN_READABLE_NAME).join(" or ")} but got ${tokenHumanReadableName} instead`,
                tokens[0]?.sourceContainer,
                tokens[0]?.location
            );
        }
    }

    checkSymbolUsage(scope, createChildScope = false) {
        this.scope = scope;

        if (createChildScope) {
            scope = scope.createChildScope();
        }

        for (var child of this.children) {
            child.checkSymbolUsage(scope);
        }
    }

    estimateTruthiness() {
        return null;
    }

    generateCode(options) {
        throw new Error("Not implemented in base class");
    }

    describe() {
        return "";
    }

    analyse() {
        var estimatedTruthiness = this.estimateTruthiness();
        var truthiness = "";

        if (estimatedTruthiness == true) {
            truthiness = "truthy";
        }

        if (estimatedTruthiness == false) {
            truthiness = "falsy";
        }

        var metadata = `${this.describe()}${truthiness}`;

        var nodeInfo = metadata.length != "" ? `${this.constructor.name} (${this.describe()}${truthiness})` : this.constructor.name;

        if (this.children.length == 0) {
            return `${nodeInfo};`;
        }

        return (
            `${nodeInfo}:\n` +
            this.children.map((child) => child.analyse()
                .split("\n")
                .map((line) => `  ${line}`)
                .join("\n")
            ).join("\n")
        );
    }
}