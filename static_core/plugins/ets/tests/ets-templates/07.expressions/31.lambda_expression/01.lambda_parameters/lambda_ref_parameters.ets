/*---
desc: lambda declaration with some parameters.
---*/

class A {}
final class B extends A {}

type func = (y: int, obj: Object, a: A) => int;

function main(): void {
  let bar: func = (y: int, obj: Object, a: A): int => { return 5; }

  assert bar(5, new Int(5), new B()) == 5;
}
