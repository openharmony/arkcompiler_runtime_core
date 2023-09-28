{% for type in topLevelTypes %}

/*---
desc: Top-level declarations
tags: [compile-only]
---*/

export {{type}}

{% endfor %}
