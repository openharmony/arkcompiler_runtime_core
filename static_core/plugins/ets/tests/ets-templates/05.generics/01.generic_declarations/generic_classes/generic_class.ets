{% for ge in generic_elements %}

/*---
desc: Declaration generic class
tags: [compile-only]
params: '{{ge}}'
---*/

class A {}

class gen{{ge}} {}

{% endfor %}
