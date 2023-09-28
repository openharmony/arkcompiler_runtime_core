/*---
desc: Subtyping relation among the class types
---*/

class A {}

final class B extends A {}


function main(): void {
  let instance: A = new B();
}
