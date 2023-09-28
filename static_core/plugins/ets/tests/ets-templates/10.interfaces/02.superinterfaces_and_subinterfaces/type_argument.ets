/*---
desc: Use well-formed parameterized type in type arguments.
assert: If a typeReference has type arguments, it must denote a well-formed parameterized type (link to parameterized types), and none of the type arguments may be wildcard type arguments, or a compile-time error occurs.
tags: [compile-only]
---*/

interface A<T> { }

interface B extends A<Boolean> { }