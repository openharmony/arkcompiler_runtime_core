{% for sd in self_dependency %}

/*---
desc: Self-dependency is prohibited
tags: [compile-only, negative]
params: >
  {{sd}}
---*/

interface X<{{sd}}> {}

function main(): void {}

{% endfor %}

