{% for wc in wrong_constraints %}

/*---
desc: wrong constraints for type variable
tags: [compile-only, negative]
params: >
  {{wc}}
---*/

function foo<X extends {{wc}}>(): void {};

function main(): void {}

{% endfor %}

