{% for type in topLevelTypes %}

/*---
desc: A few bad top-level declarations
tags: [compile-only, negative]
---*/

export default {{type}}

{% endfor %}
