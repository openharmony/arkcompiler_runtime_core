{% for type in types %}
/*---
desc: The following words cannot be used as user-defined type names but are not otherwise restricted.
tags: [compile-only]
params: '{{type}}'
---*/
let {{type}} = 0

function main(): void {}
{% endfor %}
