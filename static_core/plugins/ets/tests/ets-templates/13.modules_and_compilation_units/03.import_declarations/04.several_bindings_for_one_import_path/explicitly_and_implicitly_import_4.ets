/*---
desc: A name is explicitly used with alias and implicitly with alias.
---*/

import {sin as Sine} from "std/math";
import * as M from "std/math";

function main(): void {
  let x = Sine(1.0);
  let y = M.sin(1.0);
}