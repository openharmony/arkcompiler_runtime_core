/*---
desc: Supertype relation among array types
---*/

class A {}

final class B extends A {}

function main(): void {
  let b1 = new B();
  let b2 = new B();
  let instance: A[] = [b1, b2];
}
