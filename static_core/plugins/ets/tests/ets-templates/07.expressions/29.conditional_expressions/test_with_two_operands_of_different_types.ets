/*---
desc: Test with an object incompatible with the specified type
tags: [compile-only, negative]
---*/


function main(): void {
      let x: int = true ? 4: "hello" 
      assert x == 4;
}