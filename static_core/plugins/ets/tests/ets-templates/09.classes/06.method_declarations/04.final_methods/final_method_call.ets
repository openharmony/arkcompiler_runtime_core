/*---
desc: Checks that final method can be called.
---*/

class A {
  res: int = 0;

  final foo(): void {
    this.res = 1;
  }

}

function main(): void {
  let a: A = new A();
  a.foo();
  assert a.res == 1;
}