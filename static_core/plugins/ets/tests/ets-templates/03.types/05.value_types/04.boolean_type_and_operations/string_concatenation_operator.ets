/*---
desc: Cast from boolean to string.
---*/

function main(): void {
  let a: boolean = true;
  let b: boolean = false;

  let x = a + "str";
  let y = b + "str";

  assert x == "truestr";
  assert y == "falsestr";
}
