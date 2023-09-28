{% for wc in wrong_constraints %}

/*---
desc: wrong constraints for type variable
tags: [compile-only, negative]
params: >
  {{wc}}
---*/

interface Point<T extends {{wc}}> {}

function main(): void {}

{% endfor %}

