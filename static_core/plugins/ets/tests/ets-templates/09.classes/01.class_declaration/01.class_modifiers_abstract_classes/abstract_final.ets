/*---
desc: Class can't be both abstract and open
tags: [negative, compile-only]
---*/

abstract class A {}
final abstract class B {}

function main(): void {}
