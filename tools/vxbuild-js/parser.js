export class Token {
    constructor(source) {
        this.source = source;
    }
}

export class KeywordToken extends Token {}
export class BracketToken extends Token {}
export class DelimeterToken extends Token {}
export class StatementDelimeterToken extends Token {}
export class IdentifierToken extends Token {}

export class StringToken extends Token {
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

    function addToken(tokenClass) {
        tokens.push(new tokenClass(match[0]));
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

        if (matchToken(/^[a-zA-Z$_][a-zA-Z0-9$_]*\b/)) {
            addToken(IdentifierToken);
            continue;
        }

        if (matchToken(/^\s+/)) {
            continue;
        }

        throw new SyntaxError("Invalid syntax");
    }

    return tokens;
}