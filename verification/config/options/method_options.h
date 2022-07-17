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
        return ShowInfo[MethodOption::InfoType::CONTEXT];
    }

    bool ShowRegChanges() const
    {
        return ShowInfo[MethodOption::InfoType::REG_CHANGES];
    }

    bool ShowCflow() const
    {
        return ShowInfo[MethodOption::InfoType::CFLOW];
    }

    bool ShowJobFill() const
    {
        return ShowInfo[MethodOption::InfoType::JOBFILL];
    }

    void SetShow(MethodOption::InfoType info)
    {
        ShowInfo[info] = true;
    }

    void SetMsgClass(VerifierMessage msg_num, MethodOption::MsgClass klass)
    {
        msg_classes[msg_num][klass] = true;
    }

    template <typename Validator>
    void SetMsgClass(Validator validator, size_t msg_num, MethodOption::MsgClass klass)
    {
        if (validator(static_cast<VerifierMessage>(msg_num))) {
            msg_classes[static_cast<VerifierMessage>(msg_num)][klass] = true;
        }
    }

    void AddUpLevel(const MethodOptions &up)
    {
        uplevel.push_back(std::cref(up));
    }

    bool CanHandleMsg(VerifierMessage msg_num) const
    {
        return msg_classes.count(msg_num) > 0;
    }

    bool IsInMsgClass(VerifierMessage msg_num, MethodOption::MsgClass klass) const
    {
        if (msg_classes.count(msg_num) > 0) {
            return msg_classes.at(msg_num)[klass];
        }
        for (const auto &up : uplevel) {
            if (up.get().CanHandleMsg(msg_num)) {
                return up.get().IsInMsgClass(msg_num, klass);
            }
        }
        return false;
    }

    template <typename Handler>
    void IfInMsgClassThen(VerifierMessage msg_num, MethodOption::MsgClass klass, Handler &&handler) const
    {
        if (IsInMsgClass(msg_num, klass)) {
            handler();
        }
    }

    template <typename Handler>
    void IfNotInMsgClassThen(VerifierMessage msg_num, MethodOption::MsgClass klass, Handler &&handler) const
    {
        if (!IsInMsgClass(msg_num, klass)) {
            handler();
        }
    }

    class Proxy {
    public:
        Proxy(VerifierMessage mess_num, const MethodOptions &method_opts) : num {mess_num}, opts {method_opts} {}
        Proxy() = delete;
        Proxy(const Proxy &) = delete;
        Proxy(Proxy &&) = delete;
        ~Proxy() = default;

        bool IsError() const
        {
            return Is(MethodOption::MsgClass::ERROR);
        }

        bool IsNotError() const
        {
            return IsNot(MethodOption::MsgClass::ERROR);
        }

        bool IsWarning() const
        {
            return Is(MethodOption::MsgClass::WARNING);
        }

        bool IsNotWarning() const
        {
            return IsNot(MethodOption::MsgClass::WARNING);
        }

        template <typename Handler>
        void IfError(Handler &&handler) const
        {
            opts.IfInMsgClassThen(num, MethodOption::MsgClass::ERROR, std::move(handler));
        }

        template <typename Handler>
        void IfNotError(Handler &&handler) const
        {
            opts.IfNotInMsgClassThen(num, MethodOption::MsgClass::ERROR, std::move(handler));
        }

        template <typename Handler>
        void IfWarning(Handler &&handler) const
        {
            opts.IfInMsgClassThen(num, MethodOption::MsgClass::WARNING, std::move(handler));
        }

        template <typename Handler>
        void IfNotWarning(Handler &&handler) const
        {
            opts.IfNotInMsgClassThen(num, MethodOption::MsgClass::WARNING, std::move(handler));
        }

        template <typename Handler>
        void IfHidden(Handler &&handler) const
        {
            opts.IfInMsgClassThen(num, MethodOption::MsgClass::HIDDEN, std::move(handler));
        }

        template <typename Handler>
        void IfNotHidden(Handler &&handler) const
        {
            opts.IfNotInMsgClassThen(num, MethodOption::MsgClass::HIDDEN, std::move(handler));
        }

        bool Is(MethodOption::MsgClass klass) const
        {
            return opts.IsInMsgClass(num, klass);
        }

        bool IsNot(MethodOption::MsgClass klass) const
        {
            return !opts.IsInMsgClass(num, klass);
        }

        template <typename Handler>
        void If(MethodOption::MsgClass klass, Handler &&handler) const
        {
            opts.IfInMsgClassThen(num, klass, std::move(handler));
        }

        template <typename Handler>
        void IfNot(MethodOption::MsgClass klass, Handler &&handler) const
        {
            opts.IfNotInMsgClassThen(num, klass, std::move(handler));
        }

    private:
        const VerifierMessage num;
        const MethodOptions &opts;
    };

    Proxy Msg(VerifierMessage num) const
    {
        return {num, *this};
    }

    PandaString Image() const
    {
        PandaString result {"\n"};
        result += " Verifier messages config '" + name + "'\n";
        result += "  Uplevel configs: ";
        for (const auto &up : uplevel) {
            result += "'" + up.get().name + "' ";
        }
        result += "\n";
        result += "  Show: ";
        ShowInfo.EnumerateFlags([&](auto flag) {
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
                    result += static_cast<size_t>(flag);
                    result += ") ";
                    break;
            }
            return true;
        });
        result += "\n";
        result += "  Checks: ";
        EnabledCheck.EnumerateValues([&](auto flag) {
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
                    result += static_cast<size_t>(flag);
                    result += ") ";
                    break;
            }
            return true;
        });

        result += "\n";
        result += ImageMessages();
        return result;
    }

    MethodOptions(const PandaString &param_name) : name {param_name} {}

    const PandaString &GetName() const
    {
        return name;
    }

    MethodOption::CheckEnum &Check()
    {
        return EnabledCheck;
    }

    const MethodOption::CheckEnum &Check() const
    {
        return EnabledCheck;
    }

private:
    PandaString ImageMessages() const
    {
        PandaString result;
        result += "  Messages:\n";
        for (const auto &m : msg_classes) {
            const auto &msg_num = m.first;
            const auto &klass = m.second;
            result += "    ";
            result += VerifierMessageToString(msg_num);
            result += " : ";
            klass.EnumerateFlags([&](auto flag) {
                switch (flag) {
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
                        result += static_cast<size_t>(flag);
                        result += ")";
                        break;
                }
                return true;
            });
            result += "\n";
        }
        return result;
    }

    const PandaString name;
    PandaVector<std::reference_wrapper<const MethodOptions>> uplevel;
    PandaUnorderedMap<VerifierMessage, MethodOption::MsgClassFlag> msg_classes;
    MethodOption::InfoTypeFlag ShowInfo;
    MethodOption::CheckEnum EnabledCheck;
};

}  // namespace panda::verifier

#endif  //! PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_H_
