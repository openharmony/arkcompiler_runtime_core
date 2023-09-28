/*---
desc: Implement interface with same parameters.
assert: A class may not at the same time be a subtype of two interface types which are different parameterizations of the same generic interface, or a subtype of a parameterization of a generic interface and a raw type naming that same generic interface, or a compile-time error occurs.
tags: [compile-only, negative]
---*/

interface A<T> { }

final class D implements A<String>, A<Object> { }
