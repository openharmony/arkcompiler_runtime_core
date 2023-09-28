/*---
desc: Declare two interfaces with same parameters, A - superinterface of B, B - subinterface of A.
assert: Given a generic interface declaration I<F1,...,Fn> (n > 0), the direct superinterfaces of the parameterized interface type I<T1,...,Tn>, where Ti(1 in) is a type, are all types J<U1,...,Uk >, where J<U1,...,Uk> is a direct superinterface of I<F1,...,Fn> and is the substitution [F1:=T1,...,Fn:=Tn].
tags: [compile-only]
---*/

interface C {}

interface A<T extends C> {}

interface B<T extends C> extends A<T> {}