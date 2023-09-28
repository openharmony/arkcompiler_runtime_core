/*---
desc: Checks that final method can be called from another method inside the same class.
tags: [compile-only]
---*/

class A {
  res: int = 0;
  
  final method1(): void {
    this.res = 1;
  }
}

clss B extends A {
  method2(): void {
    this.method1();
  }
}

function main(): void {
  let a: A = new A();
  a.method2();
  assert a.res == 1;
}