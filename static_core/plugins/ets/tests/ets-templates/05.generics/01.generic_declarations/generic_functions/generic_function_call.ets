/*---
desc: parsing generic call with empty arguments list
tags: [compile-only]
---*/

class A {
	foo<U>(): void {}
	main(): void {
		this.foo<Int>()
	}
}
