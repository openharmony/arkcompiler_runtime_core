/*---
desc: Implement interface with superinterface.
assert: C has some direct superinterface J for which I is a superinterface, using the definition of "superinterface of an interface" given in link to interface’s superinterfaces.
---*/

interface I { }

interface J extends I { }

final class C implements J { }

function main(): void {
  let c: I = new C();
}
