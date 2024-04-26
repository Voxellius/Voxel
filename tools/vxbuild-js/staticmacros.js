import * as sources from "./sources.js";
import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";

export class StaticMacro {
    static NAME = null;

    constructor(astNode, namespace, closingToken = null) {
        this.astNode = astNode;
        this.namespace = namespace;
        this.closingToken = closingToken;
    }

    get arguments() {
        return this.astNode.arguments;
    }

    argumentCountRequired(count) {
        if (this.arguments.length < count) {
            throw this.generateSourceError("Missing a required argument in static macro");
        }
    }

    generateSourceError(message) {
        return new sources.SourceError(message, this.closingToken?.sourceContainer, this.closingToken?.location);
    }

    estimateTruthiness() {
        return null;
    }

    generateCode(options) {
        throw new Error("Not implemented in base class");
    }
}

export class PropertyStaticMacro extends StaticMacro {
    static NAME = "#prop";

    generateCode(options) {
        this.argumentCountRequired(1);

        var symbolName = this.arguments[0];
        var symbolCollection = namespaces.propertySymbols[symbolName];

        if (!symbolCollection || symbolCollection.length == 0) {
            return namespaces.Symbol.generateForProperty(symbolName).generateCode(options);
        }

        return symbolCollection[0].generateCode(options);
    }
}

export class UsedStaticMacro extends StaticMacro {
    static NAME = "#used";

    estimateTruthiness() {
        this.argumentCountRequired(1);

        var symbol = new namespaces.Symbol(this.namespace, this.arguments[0]);
        var usage = this.astNode.scope.symbolUses.find((usage) => usage.id == symbol.id);

        if (!usage) {
            return false;
        }

        return usage.everRead;
    }

    generateCode(options) {
        return codeGen.boolean(this.estimateTruthiness() != false);
    }
}

export class UsedPropertyStaticMacro extends StaticMacro {
    static NAME = "#usedprop";

    estimateTruthiness() {
        this.argumentCountRequired(1);

        var symbol = new namespaces.Symbol(null, this.arguments[0]);
        var usage = this.astNode.scope.symbolUses.find((usage) => usage.id == symbol.id);

        if (!usage) {
            return false;
        }

        return usage.everRead;
    }

    generateCode(options) {
        return codeGen.boolean(this.estimateTruthiness() != false);
    }
}

export class StaticMacroNode extends ast.AstNode {
    static HUMAN_READABLE_NAME = "static macro";

    static MATCH_QUERIES = [
        new ast.TokenQuery(tokeniser.StaticMacroToken)
    ];

    macroInstance = null;
    arguments = [];

    static create(tokens, namespace) {
        var instance = new this();

        var nameToken = tokens[0];
        var name = this.eat(tokens).value;
        var macroClass = STATIC_MACROS.find((macroClass) => macroClass.NAME == name);

        if (macroClass == null) {
            throw new sources.SourceError("No such static macro", nameToken?.sourceContainer, nameToken?.location);
        }

        this.eat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, "(")]);

        var addedFirstArgument = false;
        var closingToken = null;

        while (true) {
            closingToken = tokens[0];

            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.BracketToken, ")")])) {
                break;
            }

            if (addedFirstArgument) {
                this.eat(tokens, [new ast.TokenQuery(tokeniser.DelimeterToken)]);
            }

            instance.arguments.push(
                this.eat(tokens, [new ast.TokenQuery(tokeniser.IdentifierToken)]).value
            );

            addedFirstArgument = true;
        }

        instance.macroInstance = new macroClass(instance, namespace, closingToken);

        return instance;
    }

    estimateTruthiness() {
        return this.macroInstance.estimateTruthiness();
    }

    generateCode(options) {
        return this.macroInstance.generateCode(options);
    }
}

export const STATIC_MACROS = [PropertyStaticMacro, UsedStaticMacro, UsedPropertyStaticMacro];