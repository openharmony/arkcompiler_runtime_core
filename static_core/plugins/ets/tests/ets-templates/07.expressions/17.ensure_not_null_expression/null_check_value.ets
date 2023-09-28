/*---
desc: null safety -> Not-null operator
assert: For an expression e!, if the type of e is nullable, an ensure not-null expression checks whether the evaluation of e is equal to null and, if it is, throws a NullPointerException. 
---*/

function main(): void {
  let a: Int|null = 5;
  let b: Int|null = 4;
  let x: Int = a! + b!;

  assert x == 9;
}
