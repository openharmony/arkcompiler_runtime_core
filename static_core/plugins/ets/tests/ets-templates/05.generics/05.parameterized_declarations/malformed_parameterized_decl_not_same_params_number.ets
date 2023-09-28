{% for gt in generic_types %}

/*---
desc: "malformed parameterized generics: wrong amount of type parameters"
tags: [compile-only, negative]
params: >
  {{gt}}
---*/

{{gt.decl}}

function main(): void {
    let gt: {{gt.bad_param}};
}

{% endfor %}
