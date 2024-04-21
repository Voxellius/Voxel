import * as path from "https://deno.land/std@0.220.1/path/mod.ts";

const TEST_NAME = "dce";
const TEST_DIR = path.dirname(path.fromFileUrl(Deno.mainModule));
const VXC_FILE = path.join(TEST_DIR, TEST_NAME, "main.vxc");

const PASSES_TO_CHECK = [
    "if alwaysTruthy",
    "else alwaysFalsy",
    "if alwaysUnknown",
    "else alwaysUnknown",
    "if !alwaysFalsy",
    "if !alwaysUnknown",
    "if true && alwaysTruthy",
    "if alwaysTruthy && alwaysUnknown",
    "if alwaysFalsy || alwaysTruthy",
    "if alwaysFalsy || alwaysUnknown"
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
        if (!arrayContainsString(data, `"PASS ${string}\0`)) {
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

    if (arrayContainsString(data, `"FAIL\0`)) {
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