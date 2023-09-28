/*---
desc: A name is explicitly used with alias and implicitly without alias. sin is not accessible
tags: [negative]
---*/

import {sin as Sine} from "std/math";
import * from "std/math";

function main(): void {
  let x = sin(1.0);
}