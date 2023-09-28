/*---
desc: Subtyping relation among the generic class types
---*/

class A {}

final class B<T> extends A {}


function main(): void {
  let instance: A = new B<String>();
}
