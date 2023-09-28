/*---
desc: Subtyping relation among the generic class types with same parameters
---*/

class A {}
class B extends A {}
class C<T extends A> {}

final class D<T extends A> extends C<T> {}


function main(): void {
  let instance: C<B> = new D<B>();
}
