/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_H_
#define PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_H_

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "verification/util/flags.h"
#include "verification/util/saturated_enum.h"
#include "verifier_messages.h"

#include <functional>

namespace panda::verifier {

struct MethodOption {
    enum class InfoType { CONTEXT, REG_CHANGES, CFLOW, JOBFILL };
    enum class MsgClass { ERROR, WARNING, HIDDEN };
    enum class CheckType { CFLOW, RESOLVE_ID, REG_USAGE, TYPING, ABSINT };
    using InfoTypeFlag =
        FlagsForEnum<unsigned, InfoType, InfoType::CONTEXT, InfoType::REG_CHANGES, InfoType::CFLOW, InfoType::JOBFILL>;
    using MsgClassFlag = FlagsForEnum<unsigned, MsgClass, MsgClass::ERROR, MsgClass::WARNING, MsgClass::HIDDEN>;
    using CheckEnum = SaturatedEnum<CheckType, CheckType::ABSINT, CheckType::TYPING, CheckType::REG_USAGE,
                                    CheckType::RESOLVE_ID, CheckType::CFLOW>;
};

class MethodOptions {
public:
    bool ShowContext() const
    {
        return show_info_[MethodOption::InfoType::CONTEXT];
    }

    bool ShowRegChanges() const
    {
        return show_info_[MethodOption::InfoType::REG_CHANGES];
    }

    bool ShowCflow() const
    {
        return show_info_[MethodOption::InfoType::CFLOW];
    }

    bool ShowJobFill() const
    {
        return show_info_[MethodOption::InfoType::JOBFILL];
    }

    void SetShow(MethodOption::InfoType info)
    {
        show_info_[info] = true;
    }

    void SetMsgClass(VerifierMessage msg_num, MethodOption::MsgClass klass)
    {
        msg_classes_[msg_num] = klass;
    }

    template <typename Validator>
    void SetMsgClass(Validator validator, size_t msg_num, MethodOption::MsgClass klass)
    {
        if (validator(static_cast<VerifierMessage>(msg_num))) {
            msg_classes_[static_cast<VerifierMessage>(msg_num)] = klass;
        }
    }

    void AddUpLevel(const MethodOptions &up)
    {
        uplevel_.push_back(std::cref(up));
    }

    bool CanHandleMsg(VerifierMessage msg_num) const
    {
        return msg_classes_.count(msg_num) > 0;
    }

    MethodOption::MsgClass MsgClassFor(VerifierMessage msg_num) const;

    bool IsInMsgClass(VerifierMessage msg_num, MethodOption::MsgClass klass) const
    {
        return MsgClassFor(msg_num) == klass;
    }

    bool IsHidden(VerifierMessage msg_num) const
    {
        return IsInMsgClass(msg_num, MethodOption::MsgClass::HIDDEN);
    }

    bool IsWarning(VerifierMessage msg_num) const
    {
        return IsInMsgClass(msg_num, MethodOption::MsgClass::WARNING);
    }

    bool IsError(VerifierMessage msg_num) const
    {
        return IsInMsgClass(msg_num, MethodOption::MsgClass::ERROR);
    }

    PandaString Image() const
    {
        PandaString result {"\n"};
        result += " Verifier messages config '" + name_ + "'\n";
        result += "  Uplevel configs: ";
        for (const auto &up : uplevel_) {
            result += "'" + up.get().name_ + "' ";
        }
        result += "\n";
        result += "  Show: ";
        show_info_.EnumerateFlags([&](auto flag) {
            switch (flag) {
                case MethodOption::InfoType::CONTEXT:
                    result += "'context' ";
                    break;
                case MethodOption::InfoType::REG_CHANGES:
                    result += "'reg-changes' ";
                    break;
                case MethodOption::InfoType::CFLOW:
                    result += "'cflow' ";
                    break;
                case MethodOption::InfoType::JOBFILL:
                    result += "'jobfill' ";
                    break;
                default:
                    result += "<unknown>(";
                    result += std::to_string(static_cast<size_t>(flag));
                    result += ") ";
                    break;
            }
            return true;
        });
        result += "\n";
        result += "  Checks: ";
        enabled_check_.EnumerateValues([&](auto flag) {
            switch (flag) {
                case MethodOption::CheckType::TYPING:
                    result += "'typing' ";
                    break;
                case MethodOption::CheckType::ABSINT:
                    result += "'absint' ";
                    break;
                case MethodOption::CheckType::REG_USAGE:
                    result += "'reg-usage' ";
                    break;
                case MethodOption::CheckType::CFLOW:
                    result += "'cflow' ";
                    break;
                case MethodOption::CheckType::RESOLVE_ID:
                    result += "'resolve-id' ";
                    break;
                default:
                    result += "<unknown>(";
                    result += std::to_string(static_cast<size_t>(flag));
                    result += ") ";
                    break;
            }
            return true;
        });

        result += "\n";
        result += ImageMessages();
        return result;
    }

    explicit MethodOptions(PandaString param_name) : name_ {std::move(param_name)} {}

    const PandaString &GetName() const
    {
        return name_;
    }

    MethodOption::CheckEnum &Check()
    {
        return enabled_check_;
    }

    const MethodOption::CheckEnum &Check() const
    {
        return enabled_check_;
    }

private:
    PandaString ImageMessages() const
    {
        PandaString result;
        result += "  Messages:\n";
        for (const auto &m : msg_classes_) {
            const auto &msg_num = m.first;
            const auto &klass = m.second;
            result += "    ";
            result += VerifierMessageToString(msg_num);
            result += " : ";
            switch (klass) {
                case MethodOption::MsgClass::ERROR:
                    result += "E";
                    break;
                case MethodOption::MsgClass::WARNING:
                    result += "W";
                    break;
                case MethodOption::MsgClass::HIDDEN:
                    result += "H";
                    break;
                default:
                    result += "<unknown>(";
                    result += std::to_string(static_cast<size_t>(klass));
                    result += ")";
                    break;
            }
            result += "\n";
        }
        return result;
    }

    const PandaString name_;
    PandaVector<std::reference_wrapper<const MethodOptions>> uplevel_;
    PandaUnorderedMap<VerifierMessage, MethodOption::MsgClass> msg_classes_;
    MethodOption::InfoTypeFlag show_info_;
    MethodOption::CheckEnum enabled_check_;

    // In verifier_messages_data.cpp
    struct VerifierMessageDefault {
        VerifierMessage msg;
        MethodOption::MsgClass msg_class;
    };

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static VerifierMessageDefault message_defaults_[];
};

}  // namespace panda::verifier

#endif  // PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_H_
