/*---
desc: The conditional operator ?  that operator with boolean type.
---*/

function main(): void {
  let a: boolean = true;

  let x = a ? 3 : 5;

  assert x == 3;
}