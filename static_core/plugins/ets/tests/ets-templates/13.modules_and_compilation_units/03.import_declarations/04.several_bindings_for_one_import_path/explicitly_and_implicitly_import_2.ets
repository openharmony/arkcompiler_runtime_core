/*---
desc: A name is explicitly used with alias and implicitly without alias. Sine is accessible
---*/

import {sin as Sine} from "std/math";
import * from "std/math";

function main(): void {
  let x = Sine(1.0);
}