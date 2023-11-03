/*
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

#include "utils/pandargs.h"

#include <gtest/gtest.h>

namespace panda::test {
TEST(libpandargs, TestAPI)
{
    static const bool REF_DEF_BOOL = false;
    static const int REF_DEF_INT = 0;
    static const double REF_DEF_DOUBLE = 1.0;
    static const std::string REF_DEF_STRING = "noarg";
    static const uint32_t REF_DEF_UINT32 = 0;
    static const uint64_t REF_DEF_UINT64 = 0;
    static const arg_list_t REF_DEF_DLIST = arg_list_t();
    static const arg_list_t REF_DEF_LIST = arg_list_t();

    PandArg<bool> pab("bool", REF_DEF_BOOL, "Sample boolean argument");
    PandArg<int> pai("int", REF_DEF_INT, "Sample integer argument");
    PandArg<double> pad("double", REF_DEF_DOUBLE, "Sample rational argument");
    PandArg<std::string> pas("string", REF_DEF_STRING, "Sample string argument");
    PandArg<uint32_t> pau32("uint32", REF_DEF_UINT32, "Sample uint32 argument");
    PandArg<uint64_t> pau64("uint64", REF_DEF_UINT64, "Sample uint64 argument");
    PandArg<arg_list_t> pald("dlist", REF_DEF_DLIST, "Sample delimiter list argument", ":");
    PandArg<arg_list_t> pal("list", REF_DEF_LIST, "Sample list argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> pair("rint", REF_DEF_INT, "Integer argument with range", -100L, 100U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<uint32_t> paur32("ruint32", REF_DEF_UINT64, "uint32 argument with range", 0U, 1000000000U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<uint64_t> paur64("ruint64", REF_DEF_UINT64, "uint64 argument with range", 0U, 100000000000U);

    PandArgParser pa_parser;
    EXPECT_TRUE(pa_parser.Add(&pab));
    EXPECT_TRUE(pa_parser.Add(&pai));
    EXPECT_TRUE(pa_parser.Add(&pad));
    EXPECT_TRUE(pa_parser.Add(&pas));
    EXPECT_TRUE(pa_parser.Add(&pau32));
    EXPECT_TRUE(pa_parser.Add(&pau64));
    EXPECT_TRUE(pa_parser.Add(&pald));
    EXPECT_TRUE(pa_parser.Add(&pal));
    EXPECT_TRUE(pa_parser.Add(&pair));
    EXPECT_TRUE(pa_parser.Add(&paur32));
    EXPECT_TRUE(pa_parser.Add(&paur64));

    PandArg<bool> t_pab("tail_bool", REF_DEF_BOOL, "Sample tail boolean argument");
    PandArg<int> t_pai("tail_int", REF_DEF_INT, "Sample tail integer argument");
    PandArg<double> t_pad("tail_double", REF_DEF_DOUBLE, "Sample tail rational argument");
    PandArg<std::string> t_pas("tail_string", REF_DEF_STRING, "Sample tail string argument");
    PandArg<uint32_t> t_pau32("tail_uint32", REF_DEF_UINT32, "Sample tail uint32 argument");
    PandArg<uint64_t> t_pau64("tail_uint64", REF_DEF_UINT64, "Sample tail uint64 argument");

    // expect all arguments are set in parser
    {
        EXPECT_TRUE(pa_parser.IsArgSet(&pab));
        EXPECT_TRUE(pa_parser.IsArgSet(pab.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pai));
        EXPECT_TRUE(pa_parser.IsArgSet(pai.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pad));
        EXPECT_TRUE(pa_parser.IsArgSet(pad.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pas));
        EXPECT_TRUE(pa_parser.IsArgSet(pas.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pau32));
        EXPECT_TRUE(pa_parser.IsArgSet(pau32.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pau64));
        EXPECT_TRUE(pa_parser.IsArgSet(pau64.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pald));
        EXPECT_TRUE(pa_parser.IsArgSet(pald.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pal));
        EXPECT_TRUE(pa_parser.IsArgSet(pal.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&pair));
        EXPECT_TRUE(pa_parser.IsArgSet(pair.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&paur32));
        EXPECT_TRUE(pa_parser.IsArgSet(paur32.GetName()));
        EXPECT_TRUE(pa_parser.IsArgSet(&paur64));
        EXPECT_TRUE(pa_parser.IsArgSet(paur64.GetName()));
    }

    // expect default values and types are consistent
    {
        EXPECT_EQ(pab.GetDefaultValue(), REF_DEF_BOOL);
        EXPECT_EQ(pab.GetDefaultValue(), pab.GetValue());
        EXPECT_EQ(pab.GetType(), PandArgType::BOOL);

        EXPECT_EQ(pai.GetDefaultValue(), REF_DEF_INT);
        EXPECT_EQ(pai.GetDefaultValue(), pai.GetValue());
        EXPECT_EQ(pai.GetType(), PandArgType::INTEGER);

        EXPECT_DOUBLE_EQ(pad.GetValue(), REF_DEF_DOUBLE);
        EXPECT_DOUBLE_EQ(pad.GetDefaultValue(), pad.GetValue());
        EXPECT_EQ(pad.GetType(), PandArgType::DOUBLE);

        EXPECT_EQ(pas.GetDefaultValue(), REF_DEF_STRING);
        EXPECT_EQ(pas.GetDefaultValue(), pas.GetValue());
        EXPECT_EQ(pas.GetType(), PandArgType::STRING);

        EXPECT_EQ(pau32.GetDefaultValue(), REF_DEF_UINT32);
        EXPECT_EQ(pau32.GetDefaultValue(), pau32.GetValue());
        EXPECT_EQ(pau32.GetType(), PandArgType::UINT32);

        EXPECT_EQ(pau64.GetDefaultValue(), REF_DEF_UINT64);
        EXPECT_EQ(pau64.GetDefaultValue(), pau64.GetValue());
        EXPECT_EQ(pau64.GetType(), PandArgType::UINT64);

        EXPECT_TRUE(pald.GetValue().empty());
        EXPECT_EQ(pald.GetDefaultValue(), pald.GetValue());
        EXPECT_EQ(pald.GetType(), PandArgType::LIST);

        EXPECT_TRUE(pal.GetValue().empty());
        EXPECT_EQ(pal.GetDefaultValue(), pal.GetValue());
        EXPECT_EQ(pal.GetType(), PandArgType::LIST);

        EXPECT_EQ(pair.GetDefaultValue(), REF_DEF_INT);
        EXPECT_EQ(pair.GetDefaultValue(), pair.GetValue());
        EXPECT_EQ(pair.GetType(), PandArgType::INTEGER);

        EXPECT_EQ(paur32.GetDefaultValue(), REF_DEF_UINT64);
        EXPECT_EQ(paur32.GetDefaultValue(), paur32.GetValue());
        EXPECT_EQ(paur32.GetType(), PandArgType::UINT32);

        EXPECT_EQ(paur64.GetDefaultValue(), REF_DEF_UINT64);
        EXPECT_EQ(paur64.GetDefaultValue(), paur64.GetValue());
        EXPECT_EQ(paur64.GetType(), PandArgType::UINT64);
    }

    // expect false on duplicate argument
    {
        PandArg<int> pai_dup("int", 0U, "Integer number 0");
        EXPECT_TRUE(pa_parser.IsArgSet(pai_dup.GetName()));
        EXPECT_FALSE(pa_parser.Add(&pai_dup));
    }

    // add tail argument, expect false on duplicate
    // erase tail, expect 0 tail size
    {
        EXPECT_EQ(pa_parser.GetTailSize(), 0U);
        EXPECT_TRUE(pa_parser.PushBackTail(&t_pai));
        EXPECT_EQ(pa_parser.GetTailSize(), 1U);
        EXPECT_FALSE(pa_parser.PushBackTail(&t_pai));
        pa_parser.PopBackTail();
        EXPECT_EQ(pa_parser.GetTailSize(), 0U);
    }

    // expect help string formed right
    {
        std::string ref_string = "--" + pab.GetName() + ": " + pab.GetDesc() + "\n";
        ref_string += "--" + pald.GetName() + ": " + pald.GetDesc() + "\n";
        ref_string += "--" + pad.GetName() + ": " + pad.GetDesc() + "\n";
        ref_string += "--" + pai.GetName() + ": " + pai.GetDesc() + "\n";
        ref_string += "--" + pal.GetName() + ": " + pal.GetDesc() + "\n";
        ref_string += "--" + pair.GetName() + ": " + pair.GetDesc() + "\n";
        ref_string += "--" + paur32.GetName() + ": " + paur32.GetDesc() + "\n";
        ref_string += "--" + paur64.GetName() + ": " + paur64.GetDesc() + "\n";
        ref_string += "--" + pas.GetName() + ": " + pas.GetDesc() + "\n";
        ref_string += "--" + pau32.GetName() + ": " + pau32.GetDesc() + "\n";
        ref_string += "--" + pau64.GetName() + ": " + pau64.GetDesc() + "\n";
        EXPECT_EQ(pa_parser.GetHelpString(), ref_string);
    }

    // expect regular args list formed right
    {
        arg_list_t ref_arg_dlist = pald.GetValue();
        arg_list_t ref_arg_list = pal.GetValue();
        std::string ref_string = "--" + pab.GetName() + "=" + std::to_string(static_cast<int>(pab.GetValue())) + "\n";
        ref_string += "--" + pald.GetName() + "=";
        for (const auto &i : ref_arg_dlist) {
            ref_string += i + ", ";
        }
        ref_string += "\n";
        ref_string += "--" + pad.GetName() + "=" + std::to_string(pad.GetValue()) + "\n";
        ref_string += "--" + pai.GetName() + "=" + std::to_string(pai.GetValue()) + "\n";
        ref_string += "--" + pal.GetName() + "=";
        for (const auto &i : ref_arg_list) {
            ref_string += i + ", ";
        }
        ref_string += "\n";
        ref_string += "--" + pair.GetName() + "=" + std::to_string(pair.GetValue()) + "\n";
        ref_string += "--" + paur32.GetName() + "=" + std::to_string(paur32.GetValue()) + "\n";
        ref_string += "--" + paur64.GetName() + "=" + std::to_string(paur64.GetValue()) + "\n";
        ref_string += "--" + pas.GetName() + "=" + pas.GetValue() + "\n";
        ref_string += "--" + pau32.GetName() + "=" + std::to_string(pau32.GetValue()) + "\n";
        ref_string += "--" + pau64.GetName() + "=" + std::to_string(pau64.GetValue()) + "\n";
        EXPECT_EQ(pa_parser.GetRegularArgs(), ref_string);
    }

    // NOLINTBEGIN(modernize-avoid-c-arrays)
    // expect all boolean values processed right
    {
        static const char *true_values[] = {"true", "on", "1"};
        static const char *false_values[] = {"false", "off", "0"};
        static const int ARGC_BOOL_ONLY = 3;
        static const char *argv_bool_only[ARGC_BOOL_ONLY];
        argv_bool_only[0U] = "gtest_app";
        std::string s = "--" + pab.GetName();
        argv_bool_only[1U] = s.c_str();

        // NOLINTNEXTLINE(modernize-loop-convert)
        for (size_t i = 0; i < 3U; i++) {
            argv_bool_only[2U] = true_values[i];
            EXPECT_TRUE(pa_parser.Parse(ARGC_BOOL_ONLY, argv_bool_only));
            EXPECT_TRUE(pab.GetValue());
        }
        // NOLINTNEXTLINE(modernize-loop-convert)
        for (size_t i = 0; i < 3U; i++) {
            argv_bool_only[2U] = false_values[i];
            EXPECT_TRUE(pa_parser.Parse(ARGC_BOOL_ONLY, argv_bool_only));
            EXPECT_FALSE(pab.GetValue());
        }
    }

    // expect wrong boolean arguments with "=" processed right
    {
        static const int ARGC_BOOL_ONLY = 2;
        static const char *argv_bool_only[ARGC_BOOL_ONLY];
        argv_bool_only[0U] = "gtest_app";
        std::string s = "--" + pab.GetName() + "=";
        argv_bool_only[1U] = s.c_str();
        EXPECT_FALSE(pa_parser.Parse(ARGC_BOOL_ONLY, argv_bool_only));
    }

    // expect boolean at the end of arguments line is true
    {
        static const int ARGC_BOOL_ONLY = 2;
        static const char *argv_bool_only[ARGC_BOOL_ONLY];
        argv_bool_only[0U] = "gtest_app";
        std::string s = "--" + pab.GetName();
        argv_bool_only[1U] = s.c_str();
        EXPECT_TRUE(pa_parser.Parse(ARGC_BOOL_ONLY, argv_bool_only));
        EXPECT_TRUE(pab.GetValue());
    }

    // expect positive and negative integer values processed right
    {
        static const int REF_INT_POS = 42422424;
        static const int REF_INT_NEG = -42422424;
        static const int ARGC_INT_ONLY = 3;
        static const char *argv_int_only[ARGC_INT_ONLY];
        argv_int_only[0U] = "gtest_app";
        std::string s = "--" + pai.GetName();
        argv_int_only[1U] = s.c_str();
        argv_int_only[2U] = "42422424";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(pai.GetValue(), REF_INT_POS);
        argv_int_only[2U] = "-42422424";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(pai.GetValue(), REF_INT_NEG);
    }

    // expect positive and negative double values processed right
    {
        static const double REF_DOUBLE_POS = 4242.2424;
        static const double REF_DOUBLE_NEG = -4242.2424;
        static const int ARGC_DOUBLE_ONLY = 3;
        static const char *argv_double_only[ARGC_DOUBLE_ONLY];
        argv_double_only[0U] = "gtest_app";
        std::string s = "--" + pad.GetName();
        argv_double_only[1U] = s.c_str();
        argv_double_only[2U] = "4242.2424";
        EXPECT_TRUE(pa_parser.Parse(ARGC_DOUBLE_ONLY, argv_double_only));
        EXPECT_EQ(pad.GetValue(), REF_DOUBLE_POS);
        argv_double_only[2U] = "-4242.2424";
        EXPECT_TRUE(pa_parser.Parse(ARGC_DOUBLE_ONLY, argv_double_only));
        EXPECT_EQ(pad.GetValue(), REF_DOUBLE_NEG);
    }

    // expect hex values processed right
    {
        static const uint64_t REF_UINT64 = 274877906959;
        static const int REF_INT = 64;
        static const int ARGC_UINT64_INT = 3;
        static const char *argv_uint64_int[ARGC_UINT64_INT];
        argv_uint64_int[0U] = "gtest_app";
        std::string s = "--" + pau64.GetName();
        argv_uint64_int[1U] = s.c_str();
        argv_uint64_int[2U] = "0x400000000f";
        EXPECT_TRUE(pa_parser.Parse(ARGC_UINT64_INT, argv_uint64_int));
        EXPECT_EQ(pau64.GetValue(), REF_UINT64);
        argv_uint64_int[2U] = "0x40";
        EXPECT_TRUE(pa_parser.Parse(ARGC_UINT64_INT, argv_uint64_int));
        EXPECT_EQ(pau64.GetValue(), REF_INT);
    }

    // expect uint32_t values processed right
    {
        static const uint32_t REF_UINT32_POS = 4242422424;
        static const int ARGC_UINT32_ONLY = 3;
        static const char *argv_uint32_only[ARGC_UINT32_ONLY];
        argv_uint32_only[0U] = "gtest_app";
        std::string s = "--" + pau32.GetName();
        argv_uint32_only[1U] = s.c_str();
        argv_uint32_only[2U] = "4242422424";
        EXPECT_TRUE(pa_parser.Parse(ARGC_UINT32_ONLY, argv_uint32_only));
        EXPECT_EQ(pau32.GetValue(), REF_UINT32_POS);
    }

    // expect uint64_t values processed right
    {
        static const uint64_t REF_UINT64_POS = 424242422424;
        static const int ARGC_UINT64_ONLY = 3;
        static const char *argv_uint64_only[ARGC_UINT64_ONLY];
        argv_uint64_only[0U] = "gtest_app";
        std::string s = "--" + pau64.GetName();
        argv_uint64_only[1U] = s.c_str();
        argv_uint64_only[2U] = "424242422424";
        EXPECT_TRUE(pa_parser.Parse(ARGC_UINT64_ONLY, argv_uint64_only));
        EXPECT_EQ(pau64.GetValue(), REF_UINT64_POS);
    }

    // expect hex values processed right
    {
        static const uint64_t REF_UINT64 = 274877906944;
        static const int REF_INT = 64;
        static const int ARGC_UINT64_INT = 3;
        static const char *argv_uint64_int[ARGC_UINT64_INT];
        argv_uint64_int[0U] = "gtest_app";
        std::string s = "--" + pau64.GetName();
        argv_uint64_int[1U] = s.c_str();
        argv_uint64_int[2U] = "0x4000000000";
        EXPECT_TRUE(pa_parser.Parse(ARGC_UINT64_INT, argv_uint64_int));
        EXPECT_EQ(pau64.GetValue(), REF_UINT64);
        argv_uint64_int[2U] = "0x40";
        EXPECT_TRUE(pa_parser.Parse(ARGC_UINT64_INT, argv_uint64_int));
        EXPECT_EQ(pau64.GetValue(), REF_INT);
    }

    // expect out of range uint32_t values processed right
    {
        static const int ARGC_UINT32_ONLY = 3;
        static const char *argv_uint32_only[ARGC_UINT32_ONLY];
        argv_uint32_only[0U] = "gtest_app";
        std::string s = "--" + pau32.GetName();
        argv_uint32_only[1U] = s.c_str();
        argv_uint32_only[2U] = "424224244242242442422424";
        EXPECT_FALSE(pa_parser.Parse(ARGC_UINT32_ONLY, argv_uint32_only));
        argv_uint32_only[2U] = "0xffffffffffffffffffffffffff";
        EXPECT_FALSE(pa_parser.Parse(ARGC_UINT32_ONLY, argv_uint32_only));
    }

    // expect out of range uint64_t values processed right
    {
        static const int ARGC_UINT64_ONLY = 3;
        static const char *argv_uint64_only[ARGC_UINT64_ONLY];
        argv_uint64_only[0U] = "gtest_app";
        std::string s = "--" + pau64.GetName();
        argv_uint64_only[1U] = s.c_str();
        argv_uint64_only[2U] = "424224244242242442422424";
        EXPECT_FALSE(pa_parser.Parse(ARGC_UINT64_ONLY, argv_uint64_only));
        argv_uint64_only[2U] = "0xffffffffffffffffffffffffff";
        EXPECT_FALSE(pa_parser.Parse(ARGC_UINT64_ONLY, argv_uint64_only));
    }

    // expect string argument of one word and multiple word processed right
    {
        static const std::string REF_ONE_STRING = "string";
        static const std::string REF_MULTIPLE_STRING = "this is a string";
        static const char *str_argname = "--string";
        static const int ARGC_ONE_STRING = 3;
        static const char *argv_one_string[ARGC_ONE_STRING] = {"gtest_app", str_argname, "string"};
        static const int ARGC_MULTIPLE_STRING = 3;
        static const char *argv_multiple_string[ARGC_MULTIPLE_STRING] = {"gtest_app", str_argname, "this is a string"};
        EXPECT_TRUE(pa_parser.Parse(ARGC_MULTIPLE_STRING, argv_multiple_string));
        EXPECT_EQ(pas.GetValue(), REF_MULTIPLE_STRING);
        EXPECT_TRUE(pa_parser.Parse(ARGC_ONE_STRING, argv_one_string));
        EXPECT_EQ(pas.GetValue(), REF_ONE_STRING);
    }

    // expect string at the end of line is an empty string
    {
        static const int ARGC_STRING_ONLY = 2;
        static const char *argv_string_only[ARGC_STRING_ONLY];
        argv_string_only[0U] = "gtest_app";
        std::string s = "--" + pas.GetName();
        argv_string_only[1U] = s.c_str();
        EXPECT_TRUE(pa_parser.Parse(ARGC_STRING_ONLY, argv_string_only));
        EXPECT_EQ(pas.GetValue(), "");
    }

    // expect list argument processed right
    {
        pald.ResetDefaultValue();
        static const arg_list_t REF_LIST = {"list1", "list2", "list3"};
        std::string s = "--" + pald.GetName();
        static const char *list_argname = s.c_str();
        static const int ARGC_LIST_ONLY = 7;
        static const char *argv_list_only[ARGC_LIST_ONLY] = {"gtest_app", list_argname, "list1", list_argname,
                                                             "list2",     list_argname, "list3"};
        EXPECT_TRUE(pa_parser.Parse(ARGC_LIST_ONLY, argv_list_only));
        ASSERT_EQ(pald.GetValue().size(), REF_LIST.size());
        for (std::size_t i = 0; i < REF_LIST.size(); ++i) {
            EXPECT_EQ(pald.GetValue()[i], REF_LIST[i]);
        }
    }

    // expect list argument without delimiter processed right
    {
        pal.ResetDefaultValue();
        static const arg_list_t REF_LIST = {"list1", "list2", "list3", "list4"};
        std::string s = "--" + pal.GetName();
        static const char *list_argname = s.c_str();
        static const int ARGC_LIST_ONLY = 9;
        static const char *argv_list_only[ARGC_LIST_ONLY] = {
            "gtest_app", list_argname, "list1", list_argname, "list2", list_argname, "list3", list_argname, "list4"};
        EXPECT_TRUE(pa_parser.Parse(ARGC_LIST_ONLY, argv_list_only));
        ASSERT_EQ(pal.GetValue().size(), REF_LIST.size());
        for (std::size_t i = 0; i < REF_LIST.size(); ++i) {
            EXPECT_EQ(pal.GetValue()[i], REF_LIST[i]);
        }
    }

    // expect delimiter list argument processed right
    {
        pald.ResetDefaultValue();
        static const arg_list_t REF_DLIST = {"dlist1", "dlist2", "dlist3"};
        std::string s = "--" + pald.GetName();
        static const char *list_argname = s.c_str();
        static const int ARGC_DLIST_ONLY = 3;
        static const char *argv_dlist_only[ARGC_DLIST_ONLY] = {"gtest_app", list_argname, "dlist1:dlist2:dlist3"};
        EXPECT_TRUE(pa_parser.Parse(ARGC_DLIST_ONLY, argv_dlist_only));
        ASSERT_EQ(pald.GetValue().size(), REF_DLIST.size());
        for (std::size_t i = 0; i < REF_DLIST.size(); ++i) {
            EXPECT_EQ(pald.GetValue()[i], REF_DLIST[i]);
        }
    }

    // expect delimiter and multiple list argument processed right
    {
        pald.ResetDefaultValue();
        static const arg_list_t REF_LIST = {"dlist1", "dlist2", "list1", "list2", "dlist3", "dlist4"};
        std::string s = "--" + pald.GetName();
        static const char *list_argname = s.c_str();
        static const int ARGC_LIST = 9;
        static const char *argv_list[ARGC_LIST] = {"gtest_app",  list_argname, "dlist1:dlist2", list_argname,   "list1",
                                                   list_argname, "list2",      list_argname,    "dlist3:dlist4"};
        EXPECT_TRUE(pa_parser.Parse(ARGC_LIST, argv_list));
        ASSERT_EQ(pald.GetValue().size(), REF_LIST.size());
        for (std::size_t i = 0; i < REF_LIST.size(); ++i) {
            EXPECT_EQ(pald.GetValue()[i], REF_LIST[i]);
        }
    }

    // expect positive and negative integer values with range processed right
    {
        static const int REF_INT_POS = 99;
        static const int REF_INT_NEG = -99;
        static const int ARGC_INT_ONLY = 3;
        static const char *argv_int_only[ARGC_INT_ONLY];
        argv_int_only[0U] = "gtest_app";
        std::string s = "--" + pair.GetName();
        argv_int_only[1U] = s.c_str();
        argv_int_only[2U] = "99";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(pair.GetValue(), REF_INT_POS);
        argv_int_only[2U] = "-99";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(pair.GetValue(), REF_INT_NEG);
    }

    // expect wrong positive and negative integer values with range processed right
    {
        static const int REF_INT_POS = 101;
        static const int REF_INT_NEG = -101;
        static const int ARGC_INT_ONLY = 3;
        static const char *argv_int_only[ARGC_INT_ONLY];
        argv_int_only[0U] = "gtest_app";
        std::string s = "--" + pair.GetName();
        argv_int_only[1U] = s.c_str();
        argv_int_only[2U] = "101";
        EXPECT_FALSE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        argv_int_only[2U] = "-101";
        EXPECT_FALSE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
    }

    // expect uint32_t values with range processed right
    {
        static const uint32_t REF_INT_MIN = 1;
        static const uint32_t REF_INT_MAX = 990000000;
        static const int ARGC_INT_ONLY = 3;
        static const char *argv_int_only[ARGC_INT_ONLY];
        argv_int_only[0U] = "gtest_app";
        std::string s = "--" + paur32.GetName();
        argv_int_only[1U] = s.c_str();
        argv_int_only[2U] = "1";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(paur32.GetValue(), REF_INT_MIN);
        argv_int_only[2U] = "990000000";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(paur32.GetValue(), REF_INT_MAX);
    }

    // expect wrong uint32_t values with range processed right
    {
        static const uint32_t REF_INT_MIN = -1;
        static const uint32_t REF_INT_MAX = 1000000001;
        static const int ARGC_INT_ONLY = 3;
        static const char *argv_int_only[ARGC_INT_ONLY];
        argv_int_only[0U] = "gtest_app";
        std::string s = "--" + paur32.GetName();
        argv_int_only[1U] = s.c_str();
        argv_int_only[2U] = "-1";
        EXPECT_FALSE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        argv_int_only[2U] = "1000000001";
        EXPECT_FALSE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
    }

    // expect uint64_t values with range processed right
    {
        static const uint64_t REF_INT_MIN = 1;
        static const uint64_t REF_INT_MAX = 99000000000;
        static const int ARGC_INT_ONLY = 3;
        static const char *argv_int_only[ARGC_INT_ONLY];
        argv_int_only[0U] = "gtest_app";
        std::string s = "--" + paur64.GetName();
        argv_int_only[1U] = s.c_str();
        argv_int_only[2U] = "1";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(paur64.GetValue(), REF_INT_MIN);
        argv_int_only[2U] = "99000000000";
        EXPECT_TRUE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        EXPECT_EQ(paur64.GetValue(), REF_INT_MAX);
    }

    // expect wrong uint64_t values with range processed right
    {
        static const uint64_t REF_INT_MIN = -1;
        static const uint64_t REF_INT_MAX = 100000000001;
        static const int ARGC_INT_ONLY = 3;
        static const char *argv_int_only[ARGC_INT_ONLY];
        argv_int_only[0U] = "gtest_app";
        std::string s = "--" + paur64.GetName();
        argv_int_only[1U] = s.c_str();
        argv_int_only[2U] = "-1";
        EXPECT_FALSE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
        argv_int_only[2U] = "100000000001";
        EXPECT_FALSE(pa_parser.Parse(ARGC_INT_ONLY, argv_int_only));
    }

    // expect list at the end of line is a list with empty string
    {
        pald.ResetDefaultValue();
        static const arg_list_t REF_LIST = {""};
        static const int ARGC_LIST_ONLY = 2;
        static const char *argv_list_only[ARGC_LIST_ONLY];
        argv_list_only[0U] = "gtest_app";
        std::string s = "--" + pald.GetName();
        argv_list_only[1U] = s.c_str();
        EXPECT_TRUE(pa_parser.Parse(ARGC_LIST_ONLY, argv_list_only));
        EXPECT_EQ(pald.GetValue(), REF_LIST);
    }

    // expect true on IsTailEnabled when tail is enabled, false otherwise
    {
        pa_parser.EnableTail();
        EXPECT_TRUE(pa_parser.IsTailEnabled());
        pa_parser.DisableTail();
        EXPECT_FALSE(pa_parser.IsTailEnabled());
    }

    // expect tail only argument is consistent
    {
        static const int ARGC_TAIL_ONLY = 2;
        static const char *argv_tail_only[] = {"gtest_app", "tail1"};
        static const std::string REF_STR_TAIL = "tail1";
        pa_parser.EnableTail();
        pa_parser.PushBackTail(&t_pas);
        EXPECT_TRUE(pa_parser.Parse(ARGC_TAIL_ONLY, argv_tail_only));
        ASSERT_EQ(t_pas.GetValue(), REF_STR_TAIL);
        pa_parser.DisableTail();
        pa_parser.EraseTail();
    }

    // expect multiple tail only argument is consistent
    {
        static const int ARGC_TAIL_ONLY = 7;
        static const char *argv_tail_only[] = {"gtest_app", "str_tail", "off", "-4", "3.14", "2", "4"};
        static const std::string STR_REF = "str_tail";
        static const bool BOOL_REF = false;
        static const int INT_REF = -4;
        static const double DOUBLE_REF = 3.14;
        static const uint32_t UINT32_REF = 2;
        static const uint64_t UINT64_REF = 4;
        pa_parser.EnableTail();
        pa_parser.PushBackTail(&t_pas);
        pa_parser.PushBackTail(&t_pab);
        pa_parser.PushBackTail(&t_pai);
        pa_parser.PushBackTail(&t_pad);
        pa_parser.PushBackTail(&t_pau32);
        pa_parser.PushBackTail(&t_pau64);
        EXPECT_EQ(pa_parser.GetTailSize(), 6U);
        EXPECT_TRUE(pa_parser.Parse(ARGC_TAIL_ONLY, argv_tail_only));
        EXPECT_EQ(t_pas.GetValue(), STR_REF);
        EXPECT_EQ(t_pab.GetValue(), BOOL_REF);
        EXPECT_EQ(t_pai.GetValue(), INT_REF);
        EXPECT_DOUBLE_EQ(t_pad.GetValue(), DOUBLE_REF);
        EXPECT_EQ(t_pau32.GetValue(), UINT32_REF);
        EXPECT_EQ(t_pau64.GetValue(), UINT64_REF);
        pa_parser.DisableTail();
        pa_parser.EraseTail();
        EXPECT_EQ(pa_parser.GetTailSize(), 0U);
    }

    // expect parse fail on wrong tail argument type
    {
        pa_parser.EnableTail();
        static const int ARGC_TAIL_ONLY = 3;
        // boolean value instead of integer
        static const char *argv_tail_only[] = {"gtest_app", "str_tail", "off"};
        static const std::string STR_REF = "str_tail";
        pa_parser.PushBackTail(&t_pas);
        pa_parser.PushBackTail(&t_pai);
        EXPECT_EQ(pa_parser.GetTailSize(), 2U);
        EXPECT_FALSE(pa_parser.Parse(ARGC_TAIL_ONLY, argv_tail_only));
        EXPECT_EQ(t_pas.GetValue(), STR_REF);
        pa_parser.DisableTail();
        pa_parser.EraseTail();
        EXPECT_EQ(pa_parser.GetTailSize(), 0U);
    }

    // expect right tail argument processing after preceiding string argument
    {
        pa_parser.EnableTail();
        static const char *str_argname = "--string";
        static const std::string REF_STRING = "this is a reference string";
        static const std::string REF_T_STR = "string";
        static const double REF_T_DOUBLE = 0.1;
        static const bool REF_T_BOOL = true;
        static const uint32_t REF_T_UINT32 = 32;
        static const uint64_t REF_T_UINT64 = 64;
        static const int ARGC_TAIL_STRING = 8;
        static const char *argv_tail_string[] = {
            "gtest_app", str_argname, "this is a reference string", "string", ".1", "on", "32", "64"};
        pa_parser.PushBackTail(&t_pas);
        pa_parser.PushBackTail(&t_pad);
        pa_parser.PushBackTail(&t_pab);
        pa_parser.PushBackTail(&t_pau32);
        pa_parser.PushBackTail(&t_pau64);
        EXPECT_TRUE(pa_parser.Parse(ARGC_TAIL_STRING, argv_tail_string));
        EXPECT_EQ(pas.GetValue(), REF_STRING);
        EXPECT_EQ(t_pas.GetValue(), REF_T_STR);
        EXPECT_EQ(t_pad.GetValue(), REF_T_DOUBLE);
        EXPECT_EQ(t_pab.GetValue(), REF_T_BOOL);
        EXPECT_EQ(t_pau32.GetValue(), REF_T_UINT32);
        EXPECT_EQ(t_pau64.GetValue(), REF_T_UINT64);
        pa_parser.DisableTail();
        pa_parser.EraseTail();
    }

    // expect right tail argument processing after preceiding list argument
    {
        pald.ResetDefaultValue();
        pa_parser.EnableTail();
        static const char *list_argname = "--dlist";
        static const arg_list_t REF_LIST = {"list1", "list2", "list3", "list4", "list5"};
        static const double REF_T_DOUBLE = -7;
        static const bool REF_T_BOOL = true;
        static const int REF_T_INT = 255;
        static const uint32_t REF_T_UINT32 = 32;
        static const uint64_t REF_T_UINT64 = 64;
        static const int ARGC_TAIL_LIST = 16;
        static const char *argv_tail_list[] = {"gtest_app", list_argname, "list1", list_argname, "list2", list_argname,
                                               "list3",     list_argname, "list4", list_argname, "list5", "true",
                                               "255",       "-7",         "32",    "64"};
        pa_parser.PushBackTail(&t_pab);
        pa_parser.PushBackTail(&t_pai);
        pa_parser.PushBackTail(&t_pad);
        pa_parser.PushBackTail(&t_pau32);
        pa_parser.PushBackTail(&t_pau64);
        EXPECT_TRUE(pa_parser.Parse(ARGC_TAIL_LIST, argv_tail_list));
        ASSERT_EQ(pald.GetValue().size(), REF_LIST.size());
        for (std::size_t i = 0; i < REF_LIST.size(); i++) {
            EXPECT_EQ(pald.GetValue()[i], REF_LIST[i]);
        }
        EXPECT_EQ(t_pab.GetValue(), REF_T_BOOL);
        EXPECT_EQ(t_pai.GetValue(), REF_T_INT);
        EXPECT_DOUBLE_EQ(t_pad.GetValue(), REF_T_DOUBLE);
        EXPECT_EQ(t_pau32.GetValue(), REF_T_UINT32);
        EXPECT_EQ(t_pau64.GetValue(), REF_T_UINT64);

        pa_parser.DisableTail();
        pa_parser.EraseTail();
    }

    // expect right tail argument processing after noparam boolean argument
    {
        pa_parser.EnableTail();
        PandArg<std::string> t_pas0("tail_string0", REF_DEF_STRING, "Sample tail string argument 0");
        PandArg<std::string> t_pas1("tail_string1", REF_DEF_STRING, "Sample tail string argument 1");
        static const std::string REF_T_STR1 = "offtail1";
        static const std::string REF_T_STR2 = "offtail2";
        static const std::string REF_T_STR3 = "offtail3";
        static const int ARGC_TAIL_BOOL = 5;
        static const char *argv_tail_bool[] = {"gtest_app", "--bool", "offtail1", "offtail2", "offtail3"};
        pa_parser.PushBackTail(&t_pas);
        pa_parser.PushBackTail(&t_pas0);
        pa_parser.PushBackTail(&t_pas1);
        EXPECT_TRUE(pa_parser.Parse(ARGC_TAIL_BOOL, argv_tail_bool));
        EXPECT_TRUE(pab.GetValue());
        EXPECT_EQ(t_pas.GetValue(), REF_T_STR1);
        EXPECT_EQ(t_pas0.GetValue(), REF_T_STR2);
        EXPECT_EQ(t_pas1.GetValue(), REF_T_STR3);
        pa_parser.DisableTail();
        pa_parser.EraseTail();
    }

    // expect fail on amount of tail arguments more then pa_parser may have
    {
        pa_parser.EnableTail();
        static const int ARGC_TAIL = 5;
        static const char *argv_tail[] = {"gtest_app", "gdb", "--args", "file.bin", "entry"};

        PandArg<std::string> t_pas1("tail_string1", REF_DEF_STRING, "Sample tail string argument 1");
        pa_parser.PushBackTail(&t_pas);
        pa_parser.PushBackTail(&t_pas1);

        EXPECT_EQ(pa_parser.GetTailSize(), 2U);
        EXPECT_FALSE(pa_parser.Parse(ARGC_TAIL, argv_tail));
        pa_parser.DisableTail();
        pa_parser.EraseTail();
    }

    // expect remainder arguments only parsed as expected
    {
        pa_parser.EnableRemainder();
        static const arg_list_t REF_REM = {"rem1", "rem2", "rem3"};
        static int argc_rem = 5;
        static const char *argv_rem[] = {"gtest_app", "--", "rem1", "rem2", "rem3"};
        pa_parser.Parse(argc_rem, argv_rem);
        arg_list_t remainder = pa_parser.GetRemainder();
        EXPECT_EQ(remainder.size(), REF_REM.size());
        for (std::size_t i = 0; i < remainder.size(); i++) {
            EXPECT_EQ(remainder[i], REF_REM[i]);
        }
        pa_parser.DisableRemainder();
    }

    // expect regular argument before remainder parsed right
    {
        pa_parser.EnableRemainder();
        static const arg_list_t REF_REM = {"rem1", "rem2", "rem3"};
        std::string bool_name = "--" + pab.GetName();
        static int argc_rem = 6;
        static const char *argv_rem[] = {"gtest_app", bool_name.c_str(), "--", "rem1", "rem2", "rem3"};
        pa_parser.Parse(argc_rem, argv_rem);
        EXPECT_TRUE(pab.GetValue());
        arg_list_t remainder = pa_parser.GetRemainder();
        EXPECT_EQ(remainder.size(), REF_REM.size());
        for (std::size_t i = 0; i < remainder.size(); i++) {
            EXPECT_EQ(remainder[i], REF_REM[i]);
        }
        pa_parser.DisableRemainder();
    }

    // expect that all arguments parsed as expected
    {
        pald.ResetDefaultValue();
        pa_parser.EnableTail();
        pa_parser.EnableRemainder();
        static const arg_list_t REF_REM = {"rem1", "rem2", "rem3"};
        PandArg<std::string> t_pas0("tail_string0", REF_DEF_STRING, "Sample tail string argument 0");
        PandArg<std::string> t_pas1("tail_string1", REF_DEF_STRING, "Sample tail string argument 1");
        static const bool REF_BOOL = true;
        static const int REF_INT = 42;
        static const arg_list_t REF_DLIST = {"dlist1", "dlist2", "dlist3", "dlist4"};
        static const std::string REF_T_STR1 = "tail1";
        static const std::string REF_T_STR2 = "tail2 tail3";
        static const std::string REF_T_STR3 = "tail4";
        static const std::string REF_STR = "this is a string";
        static const double REF_DBL = 0.42;
        static const uint32_t REF_UINT32 = std::numeric_limits<std::uint32_t>::max();
        static const uint32_t REF_UINT32R = 990000000;
        static const uint64_t REF_UINT64 = std::numeric_limits<std::uint64_t>::max();
        static const uint64_t REF_UINT64R = 99000000000;
        // NOLINTNEXTLINE(readability-magic-numbers)
        static int argc_consistent = 21;
        static const char *argv_consistent[] = {"gtest_app",
                                                "--bool",
                                                "on",
                                                "--int=42",
                                                "--string",
                                                "this is a string",
                                                "--double",
                                                ".42",
                                                "--uint32=4294967295",
                                                "--uint64=18446744073709551615",
                                                "--dlist=dlist1:dlist2:dlist3:dlist4",
                                                "--rint=42",
                                                "--ruint32=990000000",
                                                "--ruint64=99000000000",
                                                "tail1",
                                                "tail2 tail3",
                                                "tail4",
                                                "--",
                                                "rem1",
                                                "rem2",
                                                "rem3"};
        pa_parser.PushBackTail(&t_pas);
        pa_parser.PushBackTail(&t_pas0);
        pa_parser.PushBackTail(&t_pas1);
        EXPECT_TRUE(pa_parser.Parse(argc_consistent, argv_consistent));
        EXPECT_EQ(pab.GetValue(), REF_BOOL);
        EXPECT_EQ(pai.GetValue(), REF_INT);
        EXPECT_EQ(pas.GetValue(), REF_STR);
        EXPECT_DOUBLE_EQ(pad.GetValue(), REF_DBL);
        EXPECT_EQ(pau32.GetValue(), REF_UINT32);
        EXPECT_EQ(pau64.GetValue(), REF_UINT64);
        ASSERT_EQ(pald.GetValue().size(), REF_DLIST.size());
        for (std::size_t i = 0; i < REF_DLIST.size(); ++i) {
            EXPECT_EQ(pald.GetValue()[i], REF_DLIST[i]);
        }
        EXPECT_EQ(pair.GetValue(), REF_INT);
        EXPECT_EQ(paur32.GetValue(), REF_UINT32R);
        EXPECT_EQ(paur64.GetValue(), REF_UINT64R);
        EXPECT_EQ(t_pas.GetValue(), REF_T_STR1);
        EXPECT_EQ(t_pas0.GetValue(), REF_T_STR2);
        EXPECT_EQ(t_pas1.GetValue(), REF_T_STR3);
        arg_list_t remainder = pa_parser.GetRemainder();
        EXPECT_EQ(remainder.size(), REF_REM.size());
        for (std::size_t i = 0; i < remainder.size(); i++) {
            EXPECT_EQ(remainder[i], REF_REM[i]);
        }
        pa_parser.DisableRemainder();
        pa_parser.DisableTail();
        pa_parser.EraseTail();
    }
}

TEST(libpandargs, CompoundArgs)
{
    PandArg<bool> sub_bool_arg("bool", false, "Sample boolean argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> sub_int_arg("int", 12U, "Sample integer argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<double> sub_double_arg("double", 123.45, "Sample rational argument");
    PandArg<std::string> sub_string_arg("string", "Hello", "Sample string argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> int_arg("global_int", 123U, "Global integer argument");
    PandArgCompound parent("compound", "Sample boolean argument",
                           {&sub_bool_arg, &sub_int_arg, &sub_double_arg, &sub_string_arg});

    PandArgParser pa_parser;
    ASSERT_TRUE(pa_parser.Add(&int_arg));
    ASSERT_TRUE(pa_parser.Add(&parent));

    /* Should work well with no sub arguments */
    {
        parent.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound"};
        ASSERT_TRUE(pa_parser.Parse(2U, argv)) << pa_parser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(sub_bool_arg.GetValue(), false);
        ASSERT_EQ(sub_int_arg.GetValue(), 12U);
        ASSERT_EQ(sub_double_arg.GetValue(), 123.45);
        ASSERT_EQ(sub_string_arg.GetValue(), "Hello");
    }

    {
        parent.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound:bool,int=2,double=54.321,string=World"};
        ASSERT_TRUE(pa_parser.Parse(2U, argv)) << pa_parser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(sub_bool_arg.GetValue(), true);
        ASSERT_EQ(sub_int_arg.GetValue(), 2U);
        ASSERT_EQ(sub_double_arg.GetValue(), 54.321);
        ASSERT_EQ(sub_string_arg.GetValue(), "World");
    }

    /* ResetDefaultValue should reset all sub arguments */
    {
        parent.ResetDefaultValue();
        ASSERT_EQ(parent.GetValue(), false);
        ASSERT_EQ(sub_bool_arg.GetValue(), false);
        ASSERT_EQ(sub_int_arg.GetValue(), 12U);
        ASSERT_EQ(sub_double_arg.GetValue(), 123.45);
        ASSERT_EQ(sub_string_arg.GetValue(), "Hello");
    }

    {
        static const char *argv[] = {"gtest_app", "--compound:bool=true"};
        ASSERT_TRUE(pa_parser.Parse(2U, argv)) << pa_parser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(sub_bool_arg.GetValue(), true);
    }

    {
        parent.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound:bool"};
        ASSERT_TRUE(pa_parser.Parse(2U, argv)) << pa_parser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(sub_bool_arg.GetValue(), true);
    }

    {
        static const char *argv[] = {"gtest_app", "--compound:bool=false"};
        ASSERT_TRUE(pa_parser.Parse(2U, argv)) << pa_parser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(sub_bool_arg.GetValue(), false);
    }

    {
        parent.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--global_int=321"};
        ASSERT_TRUE(pa_parser.Parse(2U, argv)) << pa_parser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), false);
        ASSERT_EQ(int_arg.GetValue(), 321U);
    }

    {
        parent.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound", "--global_int", "321"};
        ASSERT_TRUE(pa_parser.Parse(4U, argv)) << pa_parser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(int_arg.GetValue(), 321U);
    }

    /* Test that sub arguments are not visible in the global space */
    {
        static const char *argv[] = {"gtest_app", "--bool"};
        ASSERT_FALSE(pa_parser.Parse(2U, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--int=2"};
        ASSERT_FALSE(pa_parser.Parse(2U, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--double=54.321"};
        ASSERT_FALSE(pa_parser.Parse(2U, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--string=World"};
        ASSERT_FALSE(pa_parser.Parse(2U, argv));
    }
}

TEST(libpandargs, IncorrectCompoundArgs)
{
    static const arg_list_t REF_DEF_LIST = arg_list_t();
    PandArg<arg_list_t> pal("list", REF_DEF_LIST, "Sample list argument");

    PandArgParser pa_parser;
    EXPECT_TRUE(pa_parser.Add(&pal));
    pal.ResetDefaultValue();

    /* Test that incorrect using of compound argument syntax for non-compound argument produces error*/
    static const char *argv[] = {"gtest_app", "--list:list_arg1:list_arg2"};
    ASSERT_FALSE(pa_parser.Parse(2U, argv)) << pa_parser.GetErrorString();
}

// NOLINTEND(modernize-avoid-c-arrays)

}  // namespace panda::test
