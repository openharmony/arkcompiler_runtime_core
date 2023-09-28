/*---
desc: Declare two class with same parameters, A - superclass of B, B - subclass of A.
assert: Given a generic class declaration C<F1,...,Fn> (n > 0), the direct superclass of the parameterized class type C<T1,...,Tn>, where Ti (1 ≤ i ≤ n) is a type, is D<U1 θ,...,Uk θ>, where D<U1,...,Uk> is the direct superclass of C<F1,...,Fn> and θ is the substitution [F1:=T1,...,Fn:=Tn].
tags: [compile-only]
---*/

class C {}

class A<T extends Object, G extends C> {}

final class B<T extends Object, G extends C> extends A<T, G> {}
