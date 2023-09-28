{% for t in named_types_generics %}

/*---
desc: Generic bamed type {{named}}
params: {{t.type}}, {{t.value}}
tags: [compile-only]
---*/

{{t.type}}

let x: {{t.value}};


{% endfor %}