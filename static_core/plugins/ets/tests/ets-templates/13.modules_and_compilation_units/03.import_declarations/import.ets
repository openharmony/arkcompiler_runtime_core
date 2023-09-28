{% for class in classes %}
/*---
desc: Import declarations without 'as' like 'import a.b.c.d' or 'import a.b.c.*'
tags: [compile-only]
params: imported logic = {{class}}
---*/

import {{class}} from "std/math";
import * from "std/array";

{%- endfor %}
