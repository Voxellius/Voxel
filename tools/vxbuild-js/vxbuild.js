import {parseArgs} from "https://deno.land/std@0.218.2/cli/parse_args.ts";

import * as tokeniser from "./tokeniser.js";
import * as parser from "./parser.js";
import * as codeGen from "./codegen.js";

var flags = parseArgs(Deno.args, {
    string: ["input"],
    string: ["output"],
    alias: {"input": "i", "output": "o"}
});

flags["input"] ??= flags["_"].shift();

var source = await Deno.readTextFile(flags["input"]);
var tokens = tokeniser.tokenise(source);

console.log("Parsed tokens:", tokens);

var ast = parser.parse(tokens);

console.log("Built AST:", ast);

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