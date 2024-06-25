import * as sources from "./sources.js";

export class Token {
    static HUMAN_READABLE_NAME = "token";

    constructor(source) {
        this.value = source;

        this.sourceContainer = null;
        this.location = null;
    }

    static matches(token, targetValue = null) {
        if (!(token instanceof this)) {
            return false;
        }

        if (targetValue != null && token.value != targetValue) {
            return false;
        }

        return true;
    }
}

export class KeywordToken extends Token {static HUMAN_READABLE_NAME = "keyword";}
export class AtomToken extends Token {static HUMAN_READABLE_NAME = "atom";}
export class TypeLiteralToken extends Token {static HUMAN_READABLE_NAME = "type literal";}
export class StaticMacroToken extends Token {static HUMAN_READABLE_NAME = "static function name";}
export class BracketToken extends Token {static HUMAN_READABLE_NAME = "bracket";}
export class DelimeterToken extends Token {static HUMAN_READABLE_NAME = "delimeter (,)";}
export class StatementDelimeterToken extends Token {static HUMAN_READABLE_NAME = "statement delimeter (;)";}
export class PropertyAccessorToken extends Token {static HUMAN_READABLE_NAME = "property accessor (.)";}
export class PropertyDefinerToken extends Token {static HUMAN_READABLE_NAME = "property definer (:)";}
export class OperatorToken extends Token {static HUMAN_READABLE_NAME = "operator";}
export class AssignmentOperatorToken extends OperatorToken {static HUMAN_READABLE_NAME = "assignment operator";}
export class IncrementationOperatorToken extends OperatorToken {}
export class IdentifierToken extends Token {static HUMAN_READABLE_NAME = "identifier";}

export class StringToken extends Token {
    static HUMAN_READABLE_NAME = "string literal";

    constructor(value) {
        super(null);

        this.value = value;
    }
}

export class NumberToken extends Token {
    static HUMAN_READABLE_NAME = "number literal";

    constructor(value) {
        super(null);

        this.value = value;
    }
}

export function tokenise(sourceContainer) {
    var source = sourceContainer.source;
    var match = null;
    var tokens = [];
    var stringLiteralOpener = null;
    var currentString = null;
    var blockCommentDepth = 0;
    var currentPosition = 0;
    var previousPosition = 0;

    function matchToken(pattern) {
        match = source.match(pattern);

        if (match) {
            source = source.substring(match[0].length);
            previousPosition = currentPosition;
            currentPosition += match[0].length;
            
            return true;
        }
        
        return false;
    }
    
    function addToken(tokenClass, value = match[0]) {
        var token = new tokenClass(value);

        token.sourceContainer = sourceContainer;
        token.location = previousPosition;
 
        tokens.push(token);
    }

    while (source.length > 0) {
        if (stringLiteralOpener) {
            if (source[0] == stringLiteralOpener) {
                addToken(StringToken, currentString);

                source = source.substring(1);
                stringLiteralOpener = null;
                currentString = null;
                currentPosition++;

                continue;
            }

            if (matchToken(/^\\\\/)) {
                currentString += "\\";

                continue;
            }

            if (matchToken(/^\\r/)) {
                currentString += "\r";

                continue;
            }

            if (matchToken(/^\\n/)) {
                currentString += "\n";

                continue;
            }

            if (matchToken(/^\\"/)) {
                currentString += "\"";

                continue;
            }

            if (matchToken(/^\\'/)) {
                currentString += "'";

                continue;
            }

            if (matchToken(/^\\`/)) {
                currentString += "`";

                continue;
            }

            currentString += source[0];
            currentPosition++;
            source = source.substring(1);

            continue;
        }

        if (matchToken(/^\/\*/)) {
            blockCommentDepth++;

            continue;
        }

        if (blockCommentDepth > 0) {
            if (matchToken(/^\*\//)) {
                blockCommentDepth--;

                continue;
            }

            matchToken(/^(?:[^\/*]+|\/|\*)/);

            continue;
        }

        if (matchToken(/^\/\/.*?(?:\n|$)/)) {
            continue;
        }

        if (matchToken(/^["'`]/)) {
            stringLiteralOpener = match[0];
            currentString = "";
            continue;
        }

        if (matchToken(/^(?:syscall|import|as|return|function|class|extends|get|set|this|super|new|var|if|else|while|for|retain|throw|try|catch|enum|break|continue)\b/)) {
            addToken(KeywordToken);
            continue;
        }

        if (matchToken(/^(?:null|true|false)\b/)) {
            addToken(AtomToken);
            continue;
        }

        if (matchToken(/^(?:Byte|Buffer)\b/)) {
            addToken(TypeLiteralToken);
            continue;
        }

        if (matchToken(/^(?:#symbol|#usedprop|#prop|#used)\b/)) {
            addToken(StaticMacroToken);
            continue;
        }

        if (matchToken(/^[\(\{\[\)\}\]]/)) {
            addToken(BracketToken);
            continue;
        }

        if (matchToken(/^,/)) {
            addToken(DelimeterToken);
            continue;
        }

        if (matchToken(/^;/)) {
            addToken(StatementDelimeterToken);
            continue;
        }

        if (matchToken(/^\.\.\./)) {
            addToken(OperatorToken);
            continue;
        }

        if (matchToken(/^\./)) {
            addToken(PropertyAccessorToken);
            continue;
        }

        if (matchToken(/^:/)) {
            addToken(PropertyDefinerToken);
            continue;
        }

        if (matchToken(/^(?:\+\+|--)/)) {
            addToken(IncrementationOperatorToken);
            continue;
        }

        if (matchToken(/^(?:\*=|\/=|%=|\+=|-=|&&&=|\|\|\|=|&&=|\|\|=|\?\?=)/)) {
            addToken(AssignmentOperatorToken);
            continue;
        }

        if (matchToken(/^(?:<=|>=|!==|!=|===|==|&&&|\|\|\||&&|\|\||\?\?|[+\-*\/%<>!&])/)) {
            addToken(OperatorToken);
            continue;
        }

        if (matchToken(/^=/)) {
            addToken(AssignmentOperatorToken);
            continue;
        }

        if (matchToken(/^[a-zA-Z$_][a-zA-Z0-9$_]*\b/)) {
            addToken(IdentifierToken);
            continue;
        }

        if (matchToken(/^(?:[0-9]+\.?[0-9]*|[0-9]*\.?[0-9]+)(?:[eE][+-]?[0-9]+)?\b/)) {
            addToken(NumberToken, parseFloat(match[0]));
            continue;
        }

        if (matchToken(/^0b(?:[01]+)\b/)) {
            addToken(NumberToken, parseInt(match[0], 2));
            continue;
        }

        if (matchToken(/^0o(?:[0-7]+)\b/)) {
            addToken(NumberToken, parseInt(match[0], 8));
            continue;
        }

        if (matchToken(/^0x(?:[0-9a-fA-F]+)\b/)) {
            addToken(NumberToken, parseInt(match[0], 16));
            continue;
        }

        if (matchToken(/^\s+/)) {
            continue;
        }

        throw new sources.SourceError("Invalid syntax", sourceContainer, currentPosition);
    }

    return tokens;
}