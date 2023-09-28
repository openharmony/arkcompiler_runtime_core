/*---
desc: Subtyping relation among the generic interface types with same parameters
tags: [compile-only]
---*/

interface A {}
interface B {}

interface C<T extends A, G extends B> {}

interface D<T extends A, G extends B> extends C<T, G> {}
