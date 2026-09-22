/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef LIBPANDAFILE_TYPE_HELPER_H_
#define LIBPANDAFILE_TYPE_HELPER_H_

namespace ark::panda_file::helpers {

#ifndef METADATA_VERBOSE
#define METADATA_VERBOSE false
#endif

#if defined(METADATA_VERBOSE) && METADATA_VERBOSE

// CC-OFFNXT(G.PRE.02-CPP) metadata logging
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_METADATA_ENABLE() \
    const auto prevLoggerLevel = MetadataLoggerInit(Logger::Component::CUR_METADATA_LOGGER_COMPONENT)
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_METADATA_DISABLE() MetadataLoggerDestroy(Logger::Component::CUR_METADATA_LOGGER_COMPONENT, prevLoggerLevel)
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_METADATA(message) \
    LOG_INFO(CUR_METADATA_LOGGER_COMPONENT, false) << std::string((curLogLevel_)*4, ' ') << message
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_METADATA_NESTING_INC() curLogLevel_++
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_METADATA_NESTING_DEC() curLogLevel_--

inline Logger::Level MetadataLoggerInit(const Logger::Component component)
{
    ASSERT(component == Logger::Component::METADATA_SERIALIZATION ||
           component == Logger::Component::METADATA_DESERIALIZATION ||
           component == Logger::Component::METADATA_ACCESSOR);
    const auto prevLoggerLevel = Logger::IsInitialized() ? Logger::GetLevel() : Logger::Level::LAST;
    if (!Logger::IsInitialized()) {
        Logger::Initialize(logger::Options("Metadata"));
    }
    Logger::SetLevel(Logger::Level::INFO);
    Logger::EnableComponent(component);
    return prevLoggerLevel;
}

inline void MetadataLoggerDestroy(const Logger::Component component, const Logger::Level prevLoggerLevel)
{
    Logger::DisableComponent(component);
    if (prevLoggerLevel == Logger::Level::LAST) {
        Logger::Destroy();
    } else {
        Logger::SetLevel(prevLoggerLevel);
    }
}

#else

// CC-OFFNXT(G.PRE.02-CPP) metadata logging
#define LOG_METADATA_ENABLE()
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
#define LOG_METADATA_DISABLE()
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
#define LOG_METADATA(message)
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
#define LOG_METADATA_NESTING_INC()
// CC-OFFNXT(G.PRE.02-CPP) metadata logging
#define LOG_METADATA_NESTING_DEC()

#endif

}  // namespace ark::panda_file::helpers

#endif  // LIBPANDAFILE_TYPE_HELPER_H_
