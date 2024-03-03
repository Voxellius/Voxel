import * as tokeniser from "./tokeniser.js";
import * as namespaces from "./namespaces.js";
import * as statements from "./statements.js";
import * as ast from "./ast.js";

export class ModuleNode extends ast.AstNode {
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

            if (this.maybeEat(tokens, [new ast.TokenQuery(tokeniser.StatementDelimeterToken)])) {
                continue;
            }

            instance.expectChildByMatching(tokens, [statements.StatementNode], namespace);
        }

        return instance;
    }
}

export function parse(tokens) {
    return ModuleNode.create(tokens, new namespaces.Namespace());
}