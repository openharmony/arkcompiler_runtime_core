/*---
desc: lambda declaration
---*/

import * as Math from "std/math"; 

type func = (x: double) => double;

function main(): void {
  let f: func = (x: double): double => { return Math.sin(x); }
}