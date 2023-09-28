{% for named in named_types_generics %}

/*---
desc: Generic bamed type {{named}}
params: {{named}}
---*/

{{named}}

function main(): void {}

{% endfor %}