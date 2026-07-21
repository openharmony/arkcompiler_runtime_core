/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/stdlib/native/core/intl.h"
#include "plugins/ets/stdlib/native/core/intl_state.h"
#include "plugins/ets/stdlib/native/core/intl_number_format.h"
#include "plugins/ets/stdlib/native/core/intl_locale_match.h"
#include "plugins/ets/stdlib/native/core/intl_collator.h"
#include "plugins/ets/stdlib/native/core/intl_segmenter.h"
#include "plugins/ets/stdlib/native/core/intl_common.h"
#include "plugins/ets/stdlib/native/core/intl_locale.h"
#include "plugins/ets/stdlib/native/core/intl_locale_helper.h"
#include "plugins/ets/stdlib/native/core/intl_plural_rules.h"
#include "plugins/ets/stdlib/native/core/intl_date_time_format.h"
#include "plugins/ets/stdlib/native/core/intl_list_format.h"
#include "plugins/ets/stdlib/native/core/intl_relative_time_format.h"

#include "plugins/ets/stdlib/native/core/intl_display_names.h"
#include "ani/ani.h"

namespace ark::ets::stdlib::intl {

ani_status InitCoreIntl(ani_env *env)
{
    // Create internal data
    CreateIntlState();

    // Register Native methods. Stop if an error occurred
    ani_status err = RegisterIntlNumberFormatNativeMethods(env);
    err = err == ANI_OK ? RegisterIntlLocaleMatch(env) : err;
    err = err == ANI_OK ? RegisterIntlLocaleHelper(env) : err;
    err = err == ANI_OK ? RegisterIntlCollator(env) : err;
    err = err == ANI_OK ? RegisterIntlLocaleNativeMethods(env) : err;
    err = err == ANI_OK ? RegisterIntlPluralRules(env) : err;
    err = err == ANI_OK ? RegisterIntlDateTimeFormatMethods(env) : err;
    err = err == ANI_OK ? RegisterIntlRelativeTimeFormatMethods(env) : err;
    err = err == ANI_OK ? RegisterIntlSegmenter(env) : err;
    err = err == ANI_OK ? RegisterIntlListFormat(env) : err;
    err = err == ANI_OK ? RegisterIntlDisplayNames(env) : err;
    return err;
}

}  // namespace ark::ets::stdlib::intl
