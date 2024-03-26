import * as path from "https://deno.land/std@0.220.1/path/mod.ts";
import {parseArgs} from "https://deno.land/std@0.218.2/cli/parse_args.ts";

import * as sources from "./sources.js";
import * as namespaces from "./namespaces.js";
import * as tokeniser from "./tokeniser.js";
import * as parser from "./parser.js";
import * as codeGen from "./codegen.js";

try {
    var flags = parseArgs(Deno.args, {
        string: ["input"],
        string: ["output"],
        alias: {"input": "i", "output": "o"}
    });

    flags["input"] ??= flags["_"].shift();

    await namespaces.init();

    var source = await Deno.readTextFile(flags["input"]);
    var sourceContainer = new sources.SourceContainer(source, path.resolve(flags["input"]));
    var namespace = new namespaces.Namespace(sourceContainer);

    console.log("Resolving imports...");

    await namespace.resolveImports();

    var code = codeGen.join(codeGen.bytes(
        codeGen.byte("V"),
        codeGen.byte("x"),
        codeGen.byte("C"),
        1
    ), await namespace.build(!flags["no-mangle"]), codeGen.bytes(0));

    Deno.writeFile(flags["output"], code);

    console.log(`Finished building project; written to \`${flags["output"]}\``);
} catch (e) {
    if (e instanceof sources.SourceError) {
        e.log();
        Deno.exit(1);
    } else {
        throw e;
    }
}