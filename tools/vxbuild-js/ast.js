import * as sources from "./sources.js";

var nextAstId = 0;

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
        this.id = nextAstId++;
        this.parent = null;
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

            match.parent = this;

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

    findAncestorOfTypes(nodeTypes, exact = false) {
        var currentNode = this;

        while (true) {
            currentNode = currentNode.parent;

            if (currentNode == null) {
                return null;
            }

            for (var nodeType of nodeTypes) {
                if (currentNode instanceof nodeType && (!exact || currentNode.constructor == nodeType)) {
                    return currentNode;
                }
            }
        }
    }

    findDescendantsOfTypes(nodeTypes, notParentOf = []) {
        var matchingChildren = this.children.filter(function(child) {
            for (var nodeType of nodeTypes) {
                if (child instanceof nodeType) {
                    return true;
                }
            }

            return false;
        });
        
        for (var child of this.children) {
            if (notParentOf.find((parentType) => child instanceof parentType)) {
                continue;
            }

            matchingChildren.push(...child.findDescendantsOfTypes(nodeTypes, notParentOf));
        }

        return matchingChildren;
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

    pruneSymbolUsage() {
        var anyPruned = false;

        for (var child of this.children) {
            anyPruned ||= child.pruneSymbolUsage();
        }

        return anyPruned;
    }

    estimateTruthiness() {
        return null;
    }

    generateCode(options) {
        throw new Error("Not implemented in base class");
    }

    describe() {
        var estimatedTruthiness = this.estimateTruthiness();

        if (estimatedTruthiness == true) {
            return ["truthy"];
        }

        if (estimatedTruthiness == false) {
            return ["falsy"];
        }

        return [];
    }

    analyse() {
        var description = this.describe();
        var nodeInfo = description.length != "" ? `${this.constructor.name} (${description.join("; ")})` : this.constructor.name;

        if (this.children.length == 0) {
            return `${this.id} ${nodeInfo};`;
        }

        return (
            `${this.id} ${nodeInfo}:\n` +
            this.children.map((child) => child.analyse()
                .split("\n")
                .map((line) => `  ${line}`)
                .join("\n")
            ).join("\n")
        );
    }

    generateContextPath() {
        return this.parent?.generateContextPath() || "";
    }
}