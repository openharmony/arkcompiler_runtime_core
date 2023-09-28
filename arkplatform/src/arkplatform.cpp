#include <hilog/log.h>

#include "include/arkplatform.h"

namespace arkplatform {
    
void ArkPlatform::Create(const std::string& s) {
    constexpr static unsigned int ARK_DOMAIN = 0xD003F00;
    constexpr static auto TAG = "ArkPlatform";
    constexpr static OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, ARK_DOMAIN, TAG};
    OHOS::HiviewDFX::HiLog::Info(LABEL, "%{public}s", "UDAV: Hello from arkplatform");
}

}
