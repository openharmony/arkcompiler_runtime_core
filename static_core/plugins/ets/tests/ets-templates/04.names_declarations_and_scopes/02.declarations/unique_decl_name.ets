{% for un in unique_decl %}

/*---
desc: A name must be unique in its declaration space
tags: [negative, compile-only]
---*/

{{un}}
{{un}}

function main(): void {}

{% endfor %}
