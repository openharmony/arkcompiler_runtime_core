/*---
desc: lambda declaration
---*/

import * as Math from "std/math"; 

function main(): void {
  let f: (x: double) => double = (x: double): double => { return Math.sin(x); }
}