/*---
desc: Implement two interfaces, and check cast.
assert: Given a (possibly generic) class declaration C<F1,...,Fn> (n ≥ 0, C ≠ Object), the direct superinterfaces of the class type C<F1,...,Fn> are the types given in the implements clause of the declaration of C, if an implements clause is present.
---*/

interface A { }

interface B<T> { }

final class C<T extends Object> implements B<T>, A { }

function main(): void {
  let a: C<String> = new C<String>();
  let b: C<String> = new C<String>();
}
