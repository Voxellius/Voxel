import {parseArgs} from "https://deno.land/std@0.218.2/cli/parse_args.ts";

import * as parser from "./parser.js";

var flags = parseArgs(Deno.args, {
    string: ["input"],
    string: ["output"],
    alias: {"input": "i", "output": "o"}
});

flags["input"] ??= flags["_"].shift();

var source = await Deno.readTextFile(flags["input"]);

console.log(parser.tokenise(source));