/*---
desc: Methods of final class cannot be overridden
tags: [negative, compile-only]
---*/

final class A{
    methodA(): void{}
}

class B extends A{
    methodA(): void{}
}

