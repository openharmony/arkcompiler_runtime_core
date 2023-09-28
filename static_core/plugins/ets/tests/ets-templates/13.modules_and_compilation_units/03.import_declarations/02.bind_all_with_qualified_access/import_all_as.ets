{% for class in classes %}
{% for word in words %}
/*---
desc: In import declarations you cannot use '*' and 'as' at the same time
tags: [compile-only]
params: imported logic = {{class}}, identifier = {{word}}
---*/

import * as {{word}} from "std/math";

{%- endfor %}
{%- endfor %}
