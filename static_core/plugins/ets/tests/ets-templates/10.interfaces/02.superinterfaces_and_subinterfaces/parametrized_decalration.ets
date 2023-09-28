/*---
desc: Decalre interface with parameters and superinterface with same parameters.
assert: Given a (possibly generic) interface declaration I<F1,...,Fn> (n 0), the direct superinterfaces of the interface type I<F1,...,Fn> are the types given in the extends clause of the declaration of I, if an extends clause is present.
tags: [compile-only]
---*/

interface A<T, G> {}

interface B<T, G> extends A<Object, String> {}