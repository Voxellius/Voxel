export var builtinProps = {};

builtinProps["%"] = {};

builtinProps["%"]["toString"] = `
    return function(base) {
        if (base) {
            return syscall N2Sb(thing, base);
        }

        return syscall N2S(thing);
    };
`;

builtinProps["%"]["toByte"] = `
    return function() {
        return syscall N2b(thing);
    };
`;

builtinProps["%"]["isNan"] = `
    return function() {
        return syscall Nan(thing);
    };
`;

builtinProps["%"]["getEnumEntry"] = `
    return function() {
        return syscall El(thing);
    };
`;

builtinProps["B"] = {};

builtinProps["B"]["toString"] = `
    return function() {
        return syscall B2S(thing);
    };
`;

builtinProps["B"]["size"] = `
    return syscall Bz(thing);
`;

builtinProps["B"]["fill"] = `
    return function(value, start, end) {
        start ??= 0;
        end ??= syscall Bz(thing);

        while (start >= 0) {
            start = syscall Bf(thing, value, start, end);
        }
    };
`;

builtinProps["B"]["copy"] = `
    return function(destination, offset, start, end) {
        offset ??= 0;
        start ??= 0;
        end ??= syscall Bz(thing);

        while (start >= 0) {
            start = syscall Bc(thing, destination, offset, start, end);
        }
    };
`;

builtinProps["B"]["section"] = `
    return function(start, end) {
        if (start < 0 ||| end < 0) {
            var bufferSize = syscall Bz(thing);

            if (start < 0) {
                start = bufferSize + start;
            }

            if (start < 0) {
                start = 0;
            }

            if (end < 0) {
                end = bufferSize + end;
            }

            if (end < 0) {
                end = 0;
            }
        }

        if (end == null) {
            end = syscall Bz(thing);
        } else if (end < start) {
            var oldStart = start;

            start = end;
            end = oldStart;
        }

        var destination = syscall B(end - start);

        while (start >= 0) {
            start = syscall Bc(thing, destination, 0, start, end);
        }

        return destination;
    };
`;

builtinProps["\""] = {};

builtinProps["\""]["toNumber"] = `
    return function() {
        return syscall S2N(thing);
    };
`;

builtinProps["\""]["toBuffer"] = `
    return function() {
        return syscall S2B(thing);
    };
`;

builtinProps["\""]["size"] = `
    return syscall Sz(thing);
`;

builtinProps["\""]["length"] = `
    return syscall Sl(thing);
`;

builtinProps["\""]["reverse"] = `
    return function() {
        return syscall Sre(&thing);
    };
`;

builtinProps["\""]["cutStart"] = `
    return function(maxSize) {
        return syscall Scs(&thing, maxSize);
    };
`;

builtinProps["\""]["cutEnd"] = `
    return function(maxSize) {
        return syscall Sce(&thing, maxSize);
    };
`;

builtinProps["\""]["padStart"] = `
    return function(minSize, fill) {
        fill ??= " ";

        return syscall Sps(&thing, minSize, fill);
    };
`;

builtinProps["\""]["padEnd"] = `
    return function(minSize, fill) {
        fill ??= " ";

        return syscall Spe(&thing, minSize, fill);
    };
`;

builtinProps["\""]["byteAt"] = `
    return function(index) {
        return syscall Sgb(thing, index);
    };
`;

builtinProps["\""]["substring"] = `
    return function(start, end) {
        if (start < 0 ||| end < 0) {
            var stringLength = syscall Sl(thing);

            if (start < 0) {
                start = stringLength + start;
            }

            if (start < 0) {
                start = 0;
            }

            if (end < 0) {
                end = stringLength + end;
            }

            if (end < 0) {
                end = 0;
            }
        }

        if (end == null) {
            end = -1;
        } else if (end < start) {
            var oldStart = start;

            start = end;
            end = oldStart;
        }

        return syscall Sr(thing, syscall Sb(thing, start), syscall Sb(thing, end));
    };
`;

builtinProps["\""]["repeat"] = `
    return function(times) {
        var result = "";

        for (var i = 0; i < times; i++) {
            syscall Sa(result, thing);
        }

        return result;
    };
`;

builtinProps["\""]["startsWith"] = `
    return function(target) {
        return syscall Sce(&thing, syscall Sz(target)) == target;
    };
`;

builtinProps["\""]["endsWith"] = `
    return function(target) {
        return syscall Scs(&thing, syscall Sz(target)) == target;
    };
`;

builtinProps["\""]["split"] = `
    return function(delimeter) {
        var offset = 0;
        var size = syscall Sz(thing);
        var delimeterSize = syscall Sz(delimeter);
        var builtinMethods = [];
        var currentPart = "";

        while (offset < size) {
            if (syscall Sr(thing, offset, offset + delimeterSize) == delimeter) {
                syscall Lu(currentPart, builtinMethods);

                currentPart = "";
                offset += delimeterSize;

                continue;
            }

            syscall Sa(currentPart, syscall Sgb(thing, offset++));
        }

        syscall Lu(currentPart, builtinMethods);

        return builtinMethods;
    };
`;

builtinProps["L"] = {};

builtinProps["L"]["length"] = `
    return syscall Ll(thing);
`;

builtinProps["L"]["push"] = `
    return function(value) {
        return syscall Lu(value, thing);
    };
`;

builtinProps["L"]["pop"] = `
    return function() {
        return syscall Lp(thing);
    };
`;

builtinProps["L"]["unshift"] = `
    return function(value) {
        syscall Li(value, thing, 0);

        return syscall Ll(thing);
    };
`;

builtinProps["L"]["shift"] = `
    return function() {
        if (syscall Ll(thing) == 0) {
            return null;
        }

        var value = thing[0];

        syscall Lr(thing, 0);

        return value;
    };
`;

builtinProps["L"]["insert"] = `
    return function(index, value) {
        return syscall Li(value, thing, index);
    };
`;

builtinProps["L"]["removeAt"] = `
    return function(index) {
        if (index >= syscall Ll(thing)) {
            return null;
        }

        var value = thing[index];

        syscall Lr(thing, index);

        return value;
    };
`;

builtinProps["L"]["indexOf"] = `
    return function(value, equality) {
        return syscall Lf(thing, value, equality);
    };
`;

builtinProps["L"]["contains"] = `
    return function(value, equality) {
        return !!(syscall Lf(thing, value, equality) >= 0);
    };
`;

builtinProps["L"]["remove"] = `
    return function(value, equality) {
        var length = syscall Ll(thing);

        for (var i = 0; i < length; ++i) {
            if (thing[i] === value || (equality && thing[i] == value)) {
                syscall Lr(thing, i);

                return true;
            }
        }

        return false;
    };
`;

builtinProps["L"]["clear"] = `
    return function() {
        while (syscall Ll(thing) > 0) {
            syscall Lp(thing);
        }
    };
`;

builtinProps["L"]["join"] = `
    return function(delimeter) {
        delimeter ??= ",";

        return syscall Lj(thing, delimeter);
    }
`;

builtinProps["L"]["concat"] = `
    return function(source) {
        return syscall Lc(thing, source);
    }
`;

builtinProps["L"]["forEach"] = `
    return function(callback) {
        var length = syscall Ll(thing);

        for (var i = 0; i < length; ++i) {
            callback(thing[i], i);
        }
    };
`;

builtinProps["L"]["map"] = `
    return function(callback) {
        var length = syscall Ll(thing);
        var result = [];

        for (var i = 0; i < length; ++i) {
            syscall Lu(callback(thing[i], i), result);
        }

        return result;
    };
`;

builtinProps["L"]["filter"] = `
    return function(callback) {
        var length = syscall Ll(thing);
        var result = [];

        for (var i = 0; i < length; ++i) {
            var item = thing[i];

            if (callback(item, i)) {
                syscall Lu(item, result);
            }
        }

        return result;
    }
`;

builtinProps["L"]["find"] = `
    return function(callback) {
        var length = syscall Ll(thing);

        for (var i = 0; i < length; ++i) {
            var item = thing[i];

            if (callback(item, i)) {
                return item;
            }
        }

        return null;
    }
`;

builtinProps["L"]["findIndex"] = `
    return function(callback) {
        var length = syscall Ll(thing);

        for (var i = 0; i < length; ++i) {
            if (callback(thing[i], i)) {
                return i;
            }
        }

        return -1;
    }
`;

builtinProps["L"]["reduce"] = `
    return function(callback, initialValue) {
        var length = syscall Ll(thing);

        for (var i = 0; i < length; ++i) {
            initialValue = callback(initialValue, thing[i], i);
        }

        return initialValue;
    };
`;

builtinProps["O"] = {};

builtinProps["O"]["length"] = `
    return syscall Ol(thing);
`;

builtinProps["O"]["prototypes"] = `
    return syscall Op(thing);
`;

builtinProps["O"]["keyOf"] = `
    return function(value) {
        var keys = syscall Ok(thing);
        var length = syscall Ll(keys);

        for (var i = 0; i < length; ++i) {
            var key = keys[i];

            if (thing[key] == value) {
                return key;
            }
        }

        return null;
    };
`;

builtinProps["O"]["forEach"] = `
    return function(callback) {
        var keys = syscall Ok(thing);
        var length = syscall Ll(keys);

        for (var i = 0; i < length; i++) {
            var key = keys[i];

            callback(thing[key], key);
        }
    };
`;

builtinProps["O"]["map"] = `
    return function(callback) {
        var keys = syscall Ok(thing);
        var length = syscall Ll(keys);
        var result = {};

        for (var i = 0; i < length; ++i) {
            var key = keys[i];

            result[key] = callback(thing[key], key);
        }

        return result;
    };
`;

builtinProps["O"]["filter"] = `
    return function(callback) {
        var keys = syscall Ok(thing);
        var length = syscall Ll(keys);
        var result = {};

        for (var i = 0; i < length; ++i) {
            var key = keys[i];

            if (callback(thing[key], key)) {
                result[key] = thing[key];
            }
        }

        return result;
    }
`;

builtinProps["O"]["find"] = `
    return function(callback) {
        var keys = syscall Ok(thing);
        var length = syscall Ll(keys);

        for (var i = 0; i < length; ++i) {
            var key = keys[i];

            if (callback(thing[key], key)) {
                return thing[key];
            }
        }

        return null;
    }
`;

builtinProps["O"]["findKey"] = `
    return function(callback) {
        var keys = syscall Ok(thing);
        var length = syscall Ll(keys);

        for (var i = 0; i < length; ++i) {
            var key = keys[i];

            if (callback(thing[key], key)) {
                return key;
            }
        }

        return null;
    }
`;

builtinProps["O"]["reduce"] = `
    return function(callback, initialValue) {
        var keys = syscall Ok(thing);
        var length = syscall Ll(keys);

        for (var i = 0; i < length; ++i) {
            var key = keys[i];

            initialValue = callback(initialValue, thing[key], key);
        }

        return initialValue;
    };
`;

builtinProps["O"]["keys"] = `
    return syscall Ok(thing);
`;

builtinProps["O"]["values"] = `
    var values = [];
    var keys = syscall Ok(thing);
    var length = syscall Ll(keys);

    for (var i = 0; i < length; ++i) {
        syscall Lu(thing[keys[i]], values);
    }

    return values;
`;

builtinProps["O"]["pairs"] = `
    var pairs = [];
    var keys = syscall Ok(thing);
    var length = syscall Ll(keys);

    for (var i = 0; i < length; ++i) {
        var key = keys[i];

        syscall Lu({key: key, value: thing[key]}, pairs);
    }

    return pairs;
`;

builtinProps["W"] = {};

builtinProps["W"]["deref"] = `
    return function() {
        return syscall Wd(thing);
    };
`;

export var builtinPropNames = ["weak"];

Object.keys(builtinProps).forEach(function(type) {
    Object.keys(builtinProps[type]).forEach((property) => builtinPropNames.push(property));
});