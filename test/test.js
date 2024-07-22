import * as path from "https://deno.land/std@0.220.1/path/mod.ts";

const START_TIME = Date.now();
const TEST_DIR = path.dirname(path.fromFileUrl(Deno.mainModule));
const VXBUILD_FILE = path.join(TEST_DIR, "..", "tools", "vxbuild-js", "vxbuild.js");
const VOXEL_FILE = path.join(TEST_DIR, "..", "runtime", "build", "voxel");

var promises = [];

async function $(command, args) {
    var command = new Deno.Command(command, {args});
    var output = await command.output();

    return {
        code: output.code,
        stdout: new TextDecoder().decode(output.stdout),
        stderr: new TextDecoder().decode(output.stderr)
    };
}

function note(message) {
    console.log(`[${(Date.now() - START_TIME) / 1_000}] ${message}`);
}

function delay(duration) {
    return new Promise(function(resolve, reject) {
        setTimeout(function() {
            resolve();
        }, duration);
    });
}

async function measureMemoryUsage(pid, name) {
    try {
        var status = await Deno.readTextFile(path.join("/", "proc", String(pid), "status"));
    } catch (e) {
        note(`Unable to read process status file for ${name} (permissions error or crash)`);

        return -1;
    }

    return Number(status
        .split("\n")
        .find((line) => line.startsWith("VmSize:"))
        .match(/^VmSize:\s+(\d+) kB$/)[1]
    ) + Number(status
        .split("\n")
        .find((line) => line.startsWith("VmRSS:"))
        .match(/^VmRSS:\s+(\d+) kB$/)[1]
    );
}

for await (var entry of Deno.readDir(TEST_DIR)) {
    if (!entry.isDirectory) {
        continue;
    }

    const TEST_NAME = entry.name;
    const TEST_PATH = path.join(TEST_DIR, entry.name);

    var mainCode = await Deno.readTextFile(path.join(TEST_PATH, "main.vxl"));

    Deno.writeTextFile(path.join(TEST_PATH, "main.loop.vxl"), `while (true) {\n\n` + mainCode + `\n\n}`);

    promises.push(Promise.resolve().then(async function() {
        var buildOutput = await $("deno", [
            "run",
            "--allow-read",
            "--allow-write",
            VXBUILD_FILE,
            path.join(TEST_PATH, "main.vxl"),
            "-o",
            path.join(TEST_PATH, "main.vxc"),
            ...(TEST_NAME == "dce" ? ["--no-mangle"] : []) // Test relies on matching identifier names
        ]);

        if (buildOutput.stderr) {
            console.error(buildOutput.stderr);
        }

        var output = await $(VOXEL_FILE, [path.join(TEST_PATH, "main.vxc")]);

        var expected = await Deno.readTextFile(path.join(TEST_PATH, "expected.log"));

        if (output.stdout == expected) {
            note(`Test results matched expected output: ${TEST_NAME}`);
        } else {
            console.error(
                `TEST FAIL: ${TEST_NAME}\n` +
                `Expected:\n` +
                expected + `\n` +
                `Got:\n` +
                output.stdout
            );

            return Promise.resolve({
                test: TEST_NAME,
                result: "fail",
                reason: "unexpectedOutput",
                expected,
                got: output.stdout
            });
        }

        var isTestFailure = false;

        try {
            var test = await import(path.join(TEST_PATH, "test.js"));

            await test.test().catch(function(error) {
                isTestFailure = true;

                return Promise.reject(error);
            });
        } catch (e) {
            if (isTestFailure) {
                return Promise.resolve(e);
            }

            if (e.code != "ERR_MODULE_NOT_FOUND") {
                console.warn(e);
            }
        }

        await $("deno", [
            "run",
            "--allow-read",
            "--allow-write",
            VXBUILD_FILE,
            path.join(TEST_PATH, "main.loop.vxl"),
            "-o",
            path.join(TEST_PATH, "main.loop.vxc")
        ]);

        var command = new Deno.Command(VOXEL_FILE, {args: [path.join(TEST_PATH, "main.loop.vxc")], stdout: "null"});
        var process = command.spawn();

        note(`Beginning warmup for test: ${TEST_NAME}`);
        await delay(14_000);

        note(`Profiling initial memory usage for test: ${TEST_NAME}`);

        var before = await measureMemoryUsage(process.pid, TEST_NAME);

        await delay(6_000);

        note(`Profiling final memory usage for test: ${TEST_NAME}`);

        var after = await measureMemoryUsage(process.pid, TEST_NAME);

        if (before == -1 || after == -1) {
            console.error(
                `TEST FAIL: ${TEST_NAME}\n` +
                `Crash detected\n`
            );

            return Promise.resolve({
                test: TEST_NAME,
                result: "fail",
                reason: "crash",
                before,
                after
            });
        }

        process.kill();

        note(`Test process terminated: ${TEST_NAME}`);

        if (after > before + 16) {
            /*
                Give a 16 byte tolerance since memory usage could happen to be a
                bit more than beforehand but is not a memory leak
            */

            console.error(
                `TEST FAIL: ${TEST_NAME}\n` +
                `Memory leak: ${before} KiB before; ${after} KiB after (delta ${after - before} KiB)\n`
            );

            return Promise.resolve({
                test: TEST_NAME,
                result: "fail",
                reason: "memoryLeak",
                before,
                after
            });
        }

        return Promise.resolve({test: TEST_NAME, result: "pass"});
    }));
}

var results = await Promise.all(promises);
var anyFailedResult = false;

console.log("All tests completed. Summary of results:");

for (var result of results) {
    console.log(`- ${result.test}: ${result.result}`);

    if (result.result == "fail") {
        anyFailedResult = true;
    }
}

if (anyFailedResult) {
    Deno.exit(1);
}