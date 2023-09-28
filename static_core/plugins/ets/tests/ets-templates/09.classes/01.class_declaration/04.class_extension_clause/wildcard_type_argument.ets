/*---
desc: Use wildcard in type arguments.
assert: If the typeReference has type arguments, none of the type arguments may be wildcard type arguments, or a compile-time error occurs.
tags: [compile-only, negative]
---*/

class A<T> { }

class B extends A<? super Boolean> { }
