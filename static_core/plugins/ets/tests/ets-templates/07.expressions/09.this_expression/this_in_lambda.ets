/*---
desc: Keyword this in lambda.
assert: It may be used in a lambda expression only if it is allowed in the context in which the lambda expression appears.
---*/

type func = (x: int): int;

function main(): void {
  let y = 6;
  let bar: func = (x: int): int => { return this.y; };

  assert bar() == 6;
}