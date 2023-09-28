{% for ng in non_generics %}

/*---
desc: "malformed parameterized generics: use of generics with non generic"
tags: [compile-only, negative]
params: >
  {{ng}}
---*/

{{ng}}

function main(): void {
    let ng: NonGeneric<Object>;
}

{% endfor %}

