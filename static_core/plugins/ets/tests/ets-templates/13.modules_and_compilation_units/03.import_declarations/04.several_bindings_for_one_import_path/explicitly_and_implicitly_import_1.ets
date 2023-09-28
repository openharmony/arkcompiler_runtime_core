/*---
desc: A name is explicitly used without alias and implicitly with alias.
---*/

import {sin} from "std/math";
import * as Math from "std/math";

function main(): void {
  let x = sin(1.0);
  let y = Math.sin(1.0);
}