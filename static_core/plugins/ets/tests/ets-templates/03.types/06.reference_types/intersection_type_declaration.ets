/*---
desc: An intersection type is a type of the form T1 & … & Tn (n > 0), where Ti (1≤ i ≤ n) are class or interface types.
tags: [compile-only]
---*/


class A {}
interface B {}

class C<T extends A, G extends B> {}
