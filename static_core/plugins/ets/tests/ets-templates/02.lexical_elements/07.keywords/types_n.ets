{% for type in types %}
/*---
desc: The following words cannot be used as user-defined type names but are not otherwise restricted.
tags: [negative, compile-only]
params: '{{type}}'
---*/
class {{type}} {}

function main(): void {}
{% endfor %}
