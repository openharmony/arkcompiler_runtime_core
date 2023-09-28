{% for ge in generic_elements %}

/*---
desc: Declaration generic interface
tags: [compile-only]
params: '{{ge}}'
---*/

interface A {}

interface gen{{ge}} {}

{% endfor %}
