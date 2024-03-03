import {parseArgs} from "https://deno.land/std@0.218.2/cli/parse_args.ts";

import * as tokeniser from "./tokeniser.js";
import * as parser from "./parser.js";

var flags = parseArgs(Deno.args, {
    string: ["input"],
    string: ["output"],
    alias: {"input": "i", "output": "o"}
});

flags["input"] ??= flags["_"].shift();

var source = await Deno.readTextFile(flags["input"]);
var tokens = tokeniser.tokenise(source);

console.log(tokens);

var ast = parser.parse(tokens);

console.log(ast);

console.log(ast.generateCode());
console.log(new TextDecoder().decode(ast.generateCode()));