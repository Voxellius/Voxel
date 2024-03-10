export const vxcTokens = {
    NULL: byte("n"),
    BOOLEAN_TRUE: byte("t"),
    BOOLEAN_FALSE: byte("f"),
    BYTE: byte("b"),
    NUMBER_INT_8: byte("3"),
    NUMBER_INT_16: byte("4"),
    NUMBER_INT_32: byte("5"),
    NUMBER_FLOAT: byte("%"),
    BUFFER: byte("B"),
    BUFFER_EMPTY: byte("E"),
    STRING: byte("\""),
    CALL: byte("!"),
    RETURN: byte("^"),
    THROW: byte("T"),
    SET_HANDLER: byte("H"),
    CLEAR_HANDLER: byte("h"),
    GET: byte("?"),
    SET: byte(":"),
    VAR: byte("v"),
    POP: byte("p"),
    POS_REF_HERE: byte("@"),
    POS_REF_ABSOLUTE: byte("#"),
    POS_REF_BACKWARD: byte("["),
    POS_REF_FORWARD: byte("]"),
    JUMP: byte("J"),
    JUMP_IF_TRUTHY: byte("I"),
    EQUAL: byte("="),
    LESS_THAN: byte("<"),
    GREATER_THAN: byte(">"),
    NOT: byte("~"),
    AND: byte("&"),
    OR: byte("|")
};

export function byte(char) {
    return char.charCodeAt(0);
}

export function bytes(...bytes) {
    return new Uint8Array(bytes);
}

export function join(...arrays) {
    return new Uint8Array(arrays.reduce((a, b) => [...a, ...b], []));
}

export function int8(value) {
    return bytes(
        value & 0xFF
    );
}

export function int16(value) {
    return bytes(
        (bytes >> 8) & 0xFF,
        value & 0xFF
    );
}

export function int32(value) {
    return bytes(
        (bytes >> 24) & 0xFF,
        (bytes >> 16) & 0xFF,
        (bytes >> 8) & 0xFF,
        value & 0xFF
    );
}

export function number(value) {
    // TODO: Shrink depending on magnitude of value
    return join(bytes(vxcTokens.NUMBER_INT_32), int32(value));
}

export function string(value) {
    return join(
        bytes(vxcTokens.STRING),
        new TextEncoder().encode(value),
        bytes(0x00)
    );
}