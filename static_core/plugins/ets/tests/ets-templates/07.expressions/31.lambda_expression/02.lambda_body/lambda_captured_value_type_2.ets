/*---
desc: lambda captured local variable and invoke in other function. change value in labda body.
---*/

type func = (y: int) => void;

function main(): void {
  let i: int = 7;

  let bar: func = (y: int): int => { i += y; }
  bar(5);
  assert i == 12;
}