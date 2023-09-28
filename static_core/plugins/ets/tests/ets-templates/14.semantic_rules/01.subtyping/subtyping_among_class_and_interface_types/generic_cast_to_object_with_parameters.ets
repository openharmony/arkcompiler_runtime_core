/*---
desc: Subtyping relation among the generic class and Object
---*/

class A {}
final class B extends A {}

final class C<T extends A> {}

function main(): void {
  let instance: Object = new C<B>();
}
