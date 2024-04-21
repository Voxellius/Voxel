import * as tokeniser from "./tokeniser.js";
import * as namespaces from "./namespaces.js";
import * as statements from "./statements.js";
import * as ast from "./ast.js";
import * as codeGen from "./codegen.js";

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

    checkSymbolUsage() {
        this.scope = new namespaces.Scope();

        super.checkSymbolUsage(this.scope);
    }

    generateCode(options) {
        return codeGen.join(...this.children.map((child) => child.generateCode(options)));
    }
}

export function parse(tokens, namespace = new namespaces.Namespace()) {
    return ModuleNode.create(tokens, namespace);
}