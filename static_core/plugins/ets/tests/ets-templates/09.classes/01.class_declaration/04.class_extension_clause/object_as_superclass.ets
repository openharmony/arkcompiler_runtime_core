/*---
desc: Check that Object is default superclass.
assert: Given a (possibly generic) class declaration C<F1,...,Fn> (n ≥ 0, C ≠ Object), the direct superclass of the class type C<F1,...,Fn> is the type given in the extends clause of the declaration of C if an extends clause is present, or Object otherwise.
---*/

final class A<T, G> {

}

function main(): void {
  let a: Object = new A<String, Boolean>()
}
