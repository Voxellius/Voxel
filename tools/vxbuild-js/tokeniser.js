export class Token {
    static HUMAN_READABLE_NAME = "token";

    constructor(source) {
        this.value = source;
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
export class BracketToken extends Token {static HUMAN_READABLE_NAME = "bracket";}
export class DelimeterToken extends Token {static HUMAN_READABLE_NAME = "delimeter (,)";}
export class StatementDelimeterToken extends Token {static HUMAN_READABLE_NAME = "statement delimeter (;)";}
export class OperatorToken extends Token {static HUMAN_READABLE_NAME = "operator";}
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

export function tokenise(source) {
    var match = null;
    var tokens = [];
    var stringLiteralOpener = null;
    var currentString = null;

    function matchToken(pattern) {
        match = source.match(pattern);

        if (match) {
            source = source.substring(match[0].length);

            return true;
        }

        return false;
    }

    function addToken(tokenClass, value = match[0]) {
        tokens.push(new tokenClass(value));
    }

    while (source.length > 0) {
        if (stringLiteralOpener) {
            if (source[0] == stringLiteralOpener) {
                tokens.push(new StringToken(currentString));
                
                source = source.substring(1);
                stringLiteralOpener = null;
                currentString = null;

                continue;
            }

            if (matchToken(/^\\n/)) {
                currentString += "\n";

                continue;
            }

            currentString += source[0];
            source = source.substring(1);

            continue;
        }

        if (matchToken(/^["'`]/)) {
            stringLiteralOpener = match[0];
            currentString = "";
            continue;
        }

        if (matchToken(/^(function|class|var)\b/)) {
            addToken(KeywordToken);
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

        if (matchToken(/^[+\-*\/=]/)) {
            addToken(OperatorToken);
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

        if (matchToken(/^\s+/)) {
            continue;
        }

        throw new SyntaxError("Invalid syntax");
    }

    return tokens;
}