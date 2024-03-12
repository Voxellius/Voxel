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

    var source = await Deno.readTextFile(flags["input"]);
    var sourceContainer = new sources.SourceContainer(source, flags["input"]);
    var tokens = tokeniser.tokenise(sourceContainer);

    console.log("Parsed tokens:", tokens);

    var namespace = new namespaces.Namespace();
    var ast = parser.parse(tokens, namespace);

    console.log("Built AST:", ast);

    namespaces.mangleSymbols([namespace]);

    var code = codeGen.join(codeGen.bytes(
        codeGen.byte("V"),
        codeGen.byte("x"),
        codeGen.byte("C"),
        1
    ), ast.generateCode(), codeGen.bytes(0));

    console.log("Generated code:", code);
    console.log("String representation:", JSON.stringify(new TextDecoder().decode(code)));

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