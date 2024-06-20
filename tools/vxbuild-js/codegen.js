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
    SYSTEM_CALL: byte("."),
    RETURN: byte("^"),
    THROW: byte("T"),
    SET_HANDLER: byte("H"),
    CLEAR_HANDLER: byte("h"),
    GET: byte("?"),
    SET: byte(":"),
    VAR: byte("v"),
    POP: byte("p"),
    DUPE: byte("d"),
    OVER: byte("o"),
    SWAP: byte("s"),
    COPY: byte("c"),
    POS_REF_HERE: byte("@"),
    POS_REF_ABSOLUTE: byte("#"),
    POS_REF_BACKWARD: byte("["),
    POS_REF_FORWARD: byte("]"),
    JUMP: byte("J"),
    JUMP_IF_TRUTHY: byte("I"),
    IDENTICAL: byte("i"),
    EQUAL: byte("="),
    LESS_THAN: byte("<"),
    GREATER_THAN: byte(">"),
    NOT: byte("N"),
    AND: byte("A"),
    OR: byte("O")
};

export function byte(char) {
    return new TextEncoder().encode(char)[0] || 0x00;
}

export function bytes(...bytes) {
    return new Uint8Array(bytes);
}

export function join(...arrays) {
    return new Uint8Array(arrays.reduce((a, b) => [...a, ...b], []));
}

export function int8(value) {
    if (value < 0) {
        value = 0x100 + value;
    }

    return bytes(
        value & 0xFF
    );
}

export function int16(value) {
    if (value < 0) {
        value = 0x10000 + value;
    }

    return bytes(
        (value >> 8) & 0xFF,
        value & 0xFF
    );
}

export function int32(value) {
    if (value < 0) {
        value = 0x100000000 + value;
    }

    return bytes(
        (value >> 24) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF
    );
}

export function boolean(value) {
    return bytes(value ? vxcTokens.BOOLEAN_TRUE : vxcTokens.BOOLEAN_FALSE);
}

export function number(value) {
    var magnitude = Math.abs(value);

    if (magnitude <= 0x7F) {
        return join(bytes(vxcTokens.NUMBER_INT_8), int8(value));
    }

    if (magnitude <= 0x7FFF) {
        return join(bytes(vxcTokens.NUMBER_INT_16), int16(value));
    }

    return join(bytes(vxcTokens.NUMBER_INT_32), int32(value));
}

export function string(value) {
    return join(
        bytes(vxcTokens.STRING),
        new TextEncoder().encode(value),
        bytes(0x00)
    );
}

export function systemCall(value) {
    return join(
        bytes(vxcTokens.SYSTEM_CALL),
        new TextEncoder().encode(value),
        bytes(0x00)
    );
}