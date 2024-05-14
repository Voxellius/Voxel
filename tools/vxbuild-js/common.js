import * as path from "https://deno.land/std@0.220.1/path/mod.ts";

export const VXBUILD_DIR = path.dirname(path.fromFileUrl(Deno.mainModule));
export const VOXEL_DIR = path.resolve(VXBUILD_DIR, "..", "..");
export const STDLIB_DIR = path.resolve(VOXEL_DIR, "stdlib");