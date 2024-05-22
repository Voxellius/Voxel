import * as namespaces from "./namespaces.js";
import * as expressions from "./expressions.js";
import * as statements from "./statements.js";

export function hasNoEffect(astNode, applicableChildren = []) {
    for (var child of applicableChildren) {
        if (child.findDescendantsOfTypes([
            expressions.FunctionCallNode,
            expressions.PropertyAccessorNode,
            expressions.InstantiationNode
        ], [expressions.FunctionNode]).length > 0) {
            return false;
        }
    }

    var expressionNode = astNode.findAncestorOfTypes([expressions.ExpressionNode], true);

    if (expressionNode != null && expressionNode.parent instanceof statements.StatementNode) {
        return true;
    }

    return false;
}

export function markChildSymbolsAsUnread(astNode) {
    var anyMarkedUnread = false;
    var currentScope = astNode.scope;

    while (currentScope != null) {
        for (var usage of [...currentScope.symbolUses, ...currentScope.foreignSymbolUses]) {
            if (usage instanceof namespaces.ForeignSymbolUsage) {
                usage = usage.resolvedUsage;
            }

            usage.readBy = usage.readBy.filter(function(reader) {
                if (reader != astNode) {
                    return true;
                }

                console.log(usage.id || usage.name, usage.readBy.map((r) => r.generateContextPath() + (r == reader ? "[X]" : "")).join(", "));

                anyMarkedUnread = true;

                return false;
            });
        }

        currentScope = currentScope.parentScope;
    }

    for (var child of astNode.children) {
        anyMarkedUnread ||= markChildSymbolsAsUnread(child);
    }

    return anyMarkedUnread;
}