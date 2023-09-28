/*---
desc: Supertype relation among array types
---*/

class B {}

function main(): void {
  let b1 = new B();
  let b2 = new B();
  let instance: Object = [b1, b2];
}
