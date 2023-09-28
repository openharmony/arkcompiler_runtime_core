/*---
desc: Check the value assigned inside the void expression
---*/

let checkVar: int;

function foo(value: int): void {
  checkVar = value;
}

function main(): void {
  checkVar = 0;
  let checkVarMain: int = 5;
  foo(checkVarMain);
  assert checkVar == checkVarMain;
}