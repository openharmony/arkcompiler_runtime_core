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

#include <string_view>
#include "ani.h"
#include "ani_gtest.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets::ani::testing {

class MangleSignatureToProtoTest : public AniTest {
public:
    void CheckSignatureParsing(const std::string_view signature, Method::Proto &&expectedProto)
    {
        std::optional<EtsMethodSignature> methodSignature;
        Mangle::ConvertSignatureToProto(methodSignature, signature);
        ASSERT_TRUE(methodSignature.has_value()) << "Signature \"" << signature << "\" should be valid";
        EXPECT_EQ(methodSignature.value().GetProto(), expectedProto)
            << "Signature \"" << signature
            << "\" was parsed incorrectly: " << DumpProto(methodSignature.value().GetProto());
    }

    void CheckInvalidSignatureParsing(const std::string_view signature)
    {
        std::optional<EtsMethodSignature> methodSignature;
        Mangle::ConvertSignatureToProto(methodSignature, signature);
        EXPECT_FALSE(methodSignature.has_value()) << "Signature \"" << signature << "\" should be invalid";
    }

private:
    std::string DumpProto(const Method::Proto &proto)
    {
        std::stringstream ss;

        ss << "Shorty: {";
        const auto &shorty = proto.GetShorty();
        if (!shorty.empty()) {
            ss << shorty[0];
        }
        for (size_t i = 1; i < shorty.size(); ++i) {
            ss << ", " << shorty[i];
        }

        ss << "}; References: {";
        const auto &ref = proto.GetRefTypes();
        if (!ref.empty()) {
            ss << ref[0];
        }
        for (size_t i = 1; i < ref.size(); ++i) {
            ss << ", " << ref[i];
        }
        ss << "}";

        return ss.str();
    }
};

TEST_F(MangleSignatureToProtoTest, PrimitiveSignature)
{
    CheckSignatureParsing(":", Method::Proto(
                                   Method::Proto::ShortyVector {
                                       panda_file::Type {panda_file::Type::TypeId::VOID},
                                   },
                                   Method::Proto::RefTypeVector {}));

    CheckSignatureParsing("i:", Method::Proto(
                                    Method::Proto::ShortyVector {
                                        panda_file::Type {panda_file::Type::TypeId::VOID},
                                        panda_file::Type {panda_file::Type::TypeId::I32},
                                    },
                                    Method::Proto::RefTypeVector {}));

    CheckSignatureParsing("bcsilfd:z", Method::Proto(
                                           Method::Proto::ShortyVector {
                                               panda_file::Type {panda_file::Type::TypeId::U1},
                                               panda_file::Type {panda_file::Type::TypeId::I8},
                                               panda_file::Type {panda_file::Type::TypeId::U16},
                                               panda_file::Type {panda_file::Type::TypeId::I16},
                                               panda_file::Type {panda_file::Type::TypeId::I32},
                                               panda_file::Type {panda_file::Type::TypeId::I64},
                                               panda_file::Type {panda_file::Type::TypeId::F32},
                                               panda_file::Type {panda_file::Type::TypeId::F64},
                                           },
                                           Method::Proto::RefTypeVector {}));
}

TEST_F(MangleSignatureToProtoTest, ObjectsSignature)
{
    CheckSignatureParsing("C{std.core.String}i:C{T}", Method::Proto(
                                                          Method::Proto::ShortyVector {
                                                              panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                              panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                              panda_file::Type {panda_file::Type::TypeId::I32},
                                                          },
                                                          Method::Proto::RefTypeVector {
                                                              std::string_view {"LT;"},
                                                              std::string_view {"Lstd/core/String;"},
                                                          }));

    CheckSignatureParsing("sbA{l}A{C{std.core.String}}i:C{T}",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::I16},
                                  panda_file::Type {panda_file::Type::TypeId::I8},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::I32},
                              },
                              Method::Proto::RefTypeVector {
                                  std::string_view {"LT;"},
                                  std::string_view {"[J"},
                                  std::string_view {"[Lstd/core/String;"},
                              }));
}

TEST_F(MangleSignatureToProtoTest, UnionsSignature)
{
    CheckSignatureParsing("X{C{std.core.String}dC{Fun}}:",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::VOID},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {std::string_view {"{ULFun;DLstd/core/String;}"}}));

    CheckSignatureParsing(":X{C{Sun}zC{Fun}}", Method::Proto(
                                                   Method::Proto::ShortyVector {
                                                       panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                   },
                                                   Method::Proto::RefTypeVector {std::string_view {"{ULFun;LSun;Z}"}}));
}

TEST_F(MangleSignatureToProtoTest, ArraysSignature)
{
    CheckSignatureParsing("A{A{A{C{T}}}}:A{A{i}}", Method::Proto(
                                                       Method::Proto::ShortyVector {
                                                           panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                           panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                       },
                                                       Method::Proto::RefTypeVector {
                                                           std::string_view {"[[I"},
                                                           std::string_view {"[[[LT;"},
                                                       }));

    CheckSignatureParsing("lA{b}iA{c}iA{i}z:i", Method::Proto(
                                                    // CC-OFFNXT(G.FMT.06-CPP) project code style
                                                    Method::Proto::ShortyVector {
                                                        panda_file::Type {panda_file::Type::TypeId::I32},
                                                        panda_file::Type {panda_file::Type::TypeId::I64},
                                                        panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                        panda_file::Type {panda_file::Type::TypeId::I32},
                                                        panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                        panda_file::Type {panda_file::Type::TypeId::I32},
                                                        panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                                        panda_file::Type {panda_file::Type::TypeId::U1},
                                                    },
                                                    Method::Proto::RefTypeVector {
                                                        std::string_view {"[B"},
                                                        std::string_view {"[C"},
                                                        std::string_view {"[I"},
                                                    }));
}

TEST_F(MangleSignatureToProtoTest, InvalidSignature)
{
    CheckInvalidSignatureParsing("");
    CheckInvalidSignatureParsing("sbz");
    CheckInvalidSignatureParsing("{}");

    CheckInvalidSignatureParsing("i:{l}");
    CheckInvalidSignatureParsing("{i:s");
    CheckInvalidSignatureParsing(":C{T};");

    CheckInvalidSignatureParsing(":V");
    CheckInvalidSignatureParsing("::");
    CheckInvalidSignatureParsing("i:i:z");
}

TEST_F(MangleSignatureToProtoTest, NormalizePackageSeparators)
{
    CheckSignatureParsing("C{std:core.Object}:",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::VOID},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {std::string_view {"Lstd/core/Object;"}}));

    CheckSignatureParsing(":C{std:core.Object}",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {std::string_view {"Lstd/core/Object;"}}));

    CheckSignatureParsing("A{C{std:core.String}}:",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::VOID},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {std::string_view {"[Lstd/core/String;"}}));

    CheckSignatureParsing("iC{std:core.String}:z",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::U1},
                                  panda_file::Type {panda_file::Type::TypeId::I32},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {std::string_view {"Lstd/core/String;"}}));

    CheckSignatureParsing("C{std.core.Object}:",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::VOID},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {std::string_view {"Lstd/core/Object;"}}));
}

TEST_F(MangleSignatureToProtoTest, NormalizePackageSeparators_Invalid)
{
    CheckInvalidSignatureParsing("C{a/b}:");
    CheckInvalidSignatureParsing("C{a:b");
    CheckInvalidSignatureParsing("C{a:b:");
    CheckInvalidSignatureParsing("C{std:core:Object}:");
    CheckInvalidSignatureParsing("E{a:b:c.Color}:");
    CheckInvalidSignatureParsing("P{a:b:c.X}:");
    CheckInvalidSignatureParsing("E{a:b:c:d.Color}:");
    CheckInvalidSignatureParsing("P{std:foo:bar:X}:");
    CheckInvalidSignatureParsing("{:}:");
    CheckInvalidSignatureParsing("C{std:core:Object}C{a:b:c}:");
    CheckInvalidSignatureParsing("C{a:b:c}C{d:e:f}:");
    CheckInvalidSignatureParsing("A{C{a:b:c:d}}:");
}

TEST_F(MangleSignatureToProtoTest, NormalizePackageSeparators_MultipleObjects)
{
    CheckSignatureParsing("C{std:core.Object}C{std:core.String}C{std:core.Double}:",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::VOID},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {
                                  std::string_view {"Lstd/core/Object;"},
                                  std::string_view {"Lstd/core/String;"},
                                  std::string_view {"Lstd/core/Double;"},
                              }));

    CheckSignatureParsing("C{a:b.C}C{d:e.F}C{g:h.I}C{j:k.L}C{m:n.M}:i",
                          Method::Proto(
                              Method::Proto::ShortyVector {
                                  panda_file::Type {panda_file::Type::TypeId::I32},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                                  panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                              },
                              Method::Proto::RefTypeVector {
                                  std::string_view {"La/b/C;"},
                                  std::string_view {"Ld/e/F;"},
                                  std::string_view {"Lg/h/I;"},
                                  std::string_view {"Lj/k/L;"},
                                  std::string_view {"Lm/n/M;"},
                              }));
}

TEST_F(MangleSignatureToProtoTest, NormalizePackageSeparators_MultipleObjectsWithUnion)
{
    CheckSignatureParsing(
        "C{std:core.Object}C{std:core.String}C{std:core.Double}:X{A{i}C{std:core.Double}C{std:core.String}}",
        Method::Proto(
            Method::Proto::ShortyVector {
                panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                panda_file::Type {panda_file::Type::TypeId::REFERENCE},
                panda_file::Type {panda_file::Type::TypeId::REFERENCE},
            },
            Method::Proto::RefTypeVector {
                std::string_view {"{U[ILstd/core/Double;Lstd/core/String;}"},
                std::string_view {"Lstd/core/Object;"},
                std::string_view {"Lstd/core/String;"},
                std::string_view {"Lstd/core/Double;"},
            }));
}

}  // namespace ark::ets::ani::testing
