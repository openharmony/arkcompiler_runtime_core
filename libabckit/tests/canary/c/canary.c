/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../../../include/c/abckit.h"
#include "../../../include/c/statuses.h"
#include "../../../include/c/ir_core.h"
#include "../../../include/c/metadata_core.h"
#include "../../../include/c/isa/isa_dynamic.h"
#include "../../../include/c/extensions/arkts/metadata_arkts.h"
#include "../../../include/c/extensions/js/metadata_js.h"

/*
 * This a canary file to check that the public API conforms to:
 * -std=c99 -pedantic -pedantic-errors -Wall -Wextra -Werror
 */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return 0;
}
