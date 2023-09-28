/*---
desc: Implement interface with same parameters.
tags: [compile-only]
---*/

interface A<T> { }

interface B extends A<String> { }

interface C extends A<Int> { }

final class D implements B, C { }
