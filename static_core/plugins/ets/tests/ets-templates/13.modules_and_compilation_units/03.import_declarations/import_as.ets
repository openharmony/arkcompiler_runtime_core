{% for class in classes %}
{% for word in words %}
/*---
desc: Import declarations with 'as' like 'import a.b.c.d as name'
tags: [compile-only]
params: imported logic = {{class}}, identifier = {{word}}
---*/

import { {{class}} as {{word}} } from "std/math";

{% endfor %}
{% endfor %}
