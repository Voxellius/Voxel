import * as path from "https://deno.land/std@0.220.1/path/mod.ts";

const TEST_NAME = "dce";
const TEST_DIR = path.dirname(path.fromFileUrl(Deno.mainModule));
const VXC_FILE = path.join(TEST_DIR, TEST_NAME, "main.vxc");

function stringCheck(string) {
    return `"${string}\0`;
}

function variableDefinitionCheck(name) {
    return `:${name}\0v`;
}

function propertyDefinitionCheck(name) {
    return stringCheck(name);
}

const PASSES_TO_CHECK = [
    stringCheck("PASS if falsyToStartWith"),
    stringCheck("PASS else falsyToStartWith"),
    stringCheck("PASS if alwaysTruthy"),
    stringCheck("PASS else alwaysFalsy"),
    stringCheck("PASS if alwaysUnknown"),
    stringCheck("PASS else alwaysUnknown"),
    stringCheck("PASS if !alwaysFalsy"),
    stringCheck("PASS if !alwaysUnknown"),
    stringCheck("PASS if true && alwaysTruthy"),
    stringCheck("PASS if alwaysTruthy && alwaysUnknown"),
    stringCheck("PASS if alwaysFalsy || alwaysTruthy"),
    stringCheck("PASS if alwaysFalsy || alwaysUnknown"),
    variableDefinitionCheck("usedC_PASS"),
    variableDefinitionCheck("usedB_PASS"),
    variableDefinitionCheck("usedA_PASS"),
    stringCheck("PASS usedC"),
    stringCheck("PASS if #used(usedA_PASS)"),
    stringCheck("main:SideEffectClass_PASS"),
    stringCheck("main:UsedClass_PASS"),
    propertyDefinitionCheck("usedProp_PASS"),
    propertyDefinitionCheck("usedMethod_PASS")
];

function bytesMatchAt(array, bytes, index) {
    for (var i = 0; i < bytes.length; i++) {
        if (array[index++] != bytes[i]) {
            return false;
        }
    }

    return true;
}

function arrayContainsBytes(array, bytes) {
    for (var i = 0; i < array.length; i++) {
        if (bytesMatchAt(array, bytes, i)) {
            return true;
        }
    }

    return false;
}

function arrayContainsString(array, string) {
    return arrayContainsBytes(array, new TextEncoder().encode(string));
}

export async function test() {
    var data = await Deno.readFile(VXC_FILE);

    for (var string of PASSES_TO_CHECK) {
        if (!arrayContainsString(data, string)) {
            console.error(
                `TEST FAIL: ${TEST_NAME}\n` +
                `Expected in VxC output but never matched: \`${string}\``
            );

            return Promise.reject({
                test: TEST_NAME,
                result: "fail",
                reason: "noMatch",
                string
            });
        }
    }

    if (arrayContainsString(data, `"FAIL\0`) || arrayContainsString(data, "FAIL\0v")) {
        console.error(
            `TEST FAIL: ${TEST_NAME}\n` +
            `Encountered a failure trap in VxC output`
        );

        return Promise.reject({
            test: TEST_NAME,
            result: "fail",
            reason: "matchFailureTrap"
        });
    }

    return Promise.resolve();
}