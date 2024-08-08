/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_PARSER_H
#define PANDA_ASSEMBLER_ASSEMBLY_PARSER_H

#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "assembly-context.h"
#include "assembly-emitter.h"
#include "assembly-field.h"
#include "assembly-function.h"
#include "assembly-ins.h"
#include "assembly-label.h"
#include "assembly-program.h"
#include "assembly-record.h"
#include "assembly-type.h"
#include "define.h"
#include "error.h"
#include "ide_helpers.h"
#include "lexer.h"
#include "meta.h"
#include "utils/expected.h"

namespace ark::pandasm {

using Instructions = std::pair<std::vector<Ins>, Error>;

using Functions = std::pair<std::unordered_map<std::string, Function>, std::unordered_map<std::string, Record>>;

class Parser {
public:
    Parser() = default;

    NO_MOVE_SEMANTIC(Parser);
    NO_COPY_SEMANTIC(Parser);

    ~Parser() = default;

    /*
     * The main function of parsing, which takes a vector of token vectors and a name of the source file.
     * Returns a program or an error value: Expected<Program, Error>
     * This function analyzes code containing several functions:
     *   - Each function used must be declared.
     *   - The correct function declaration looks like this: .function ret_type fun_name([param_type aN,]) [<metadata>]
     *     ([data] shows that this 'data' is optional).
     *   - N in function parameters must increase when number of parameters increases
     *     (Possible: a0, a1,..., aN. Impossible: a1, a10, a13).
     *   - Each function has its own label table.
     */
    PANDA_PUBLIC_API Expected<Program, Error> Parse(TokenSet &vectorsTokens, const std::string &fileName = "");

    /*
     * The main function of parsing, which takes a string with source and a name of the source file.
     * Returns a program or an error value: Expected<Program, Error>
     */
    PANDA_PUBLIC_API Expected<Program, Error> Parse(const std::string &source, const std::string &fileName = "");

    /*
     * Returns a set error
     */
    Error ShowError() const
    {
        return err_;
    }

    ErrorList ShowWarnings() const
    {
        return war_;
    }

private:
    ark::pandasm::Program program_;
    std::unordered_map<std::string, ark::pandasm::Label> *labelTable_ = nullptr;
    Metadata *metadata_ = nullptr;
    Context context_; /* token iterator */
    ark::pandasm::Record *currRecord_ = nullptr;
    ark::pandasm::LiteralArray *currArray_ = nullptr;
    bool isConstArray_ = false;
    ark::pandasm::LiteralArray::Literal *currArrayElem_ = nullptr;
    ark::pandasm::Function *currFunc_ = nullptr;
    ark::pandasm::Ins *currIns_ = nullptr;
    ark::pandasm::Field *currFld_ = nullptr;
    size_t lineStric_ = 0;
    ark::pandasm::Error err_;
    ark::pandasm::ErrorList war_;
    bool open_ = false; /* flag of being in a code section */
    bool recordDef_ = false;
    bool arrayDef_ = false;
    bool funcDef_ = false;
    static constexpr uint32_t INTRO_CONST_ARRAY_LITERALS_NUMBER = 2;

    inline Error GetError(const std::string &mess = "", Error::ErrorType err = Error::ErrorType::ERR_NONE,
                          int8_t shift = 0, int tokenShift = 0, const std::string &addMess = "") const
    {
        return Error(mess, lineStric_, err, addMess,
                     context_.tokens[static_cast<int>(context_.number) + tokenShift - 1].boundLeft + shift,
                     context_.tokens[static_cast<int>(context_.number) + tokenShift - 1].boundRight,
                     context_.tokens[static_cast<int>(context_.number) + tokenShift - 1].wholeLine);
    }

    inline void GetWarning(const std::string &mess = "", Error::ErrorType err = Error::ErrorType::ERR_NONE,
                           int8_t shift = 0, const std::string &addMess = "")
    {
        war_.emplace_back(mess, lineStric_, err, addMess,
                          context_.tokens[context_.number - 1].boundLeft + static_cast<size_t>(shift),
                          context_.tokens[context_.number - 1].boundRight,
                          context_.tokens[context_.number - 1].wholeLine, Error::ErrorClass::WARNING);
    }

    SourcePosition GetCurrentPosition(bool leftBound) const
    {
        if (leftBound) {
            return SourcePosition {lineStric_, context_.tokens[context_.number - 1].boundLeft};
        }
        return SourcePosition {lineStric_, context_.tokens[context_.number - 1].boundRight};
    }

    bool LabelValidName();
    bool TypeValidName();
    bool RegValidName();
    bool ParamValidName();
    bool FunctionValidName();
    bool ParseFunctionName();
    bool ParseLabel();
    bool ParseOperation();
    bool ParseOperands();
    bool ParseFunctionCode();
    bool ParseFunctionInstruction();
    bool ParseFunctionFullSign();
    bool UpdateFunctionName();
    bool ParseFunctionReturn();
    bool ParseFunctionArg();
    bool ParseFunctionArgComma(bool &comma);
    bool ParseFunctionArgs();
    bool ParseType(Type *type);
    bool PrefixedValidName(bool allowBrackets = false);
    bool ParseMetaListComma(bool &comma, bool eq);
    bool MeetExpMetaList(bool eq);
    bool BuildMetaListAttr(bool &eq, std::string &attributeName, std::string &attributeValue);
    bool ParseMetaList(bool flag);
    bool ParseMetaDef();
    bool ParseRecordFullSign();
    bool ParseRecordFields();
    bool ParseRecordField();
    bool ParseRecordName();
    bool RecordValidName();
    bool ParseArrayFullSign();
    bool IsConstArray();
    bool ParseArrayName();
    bool ArrayValidName();
    bool ArrayElementsValidNumber();
    bool ParseArrayElements();
    bool ParseArrayElement();
    bool ParseArrayElementType();
    bool ParseArrayElementValue();
    bool ParseArrayElementValueInteger();
    bool ParseArrayElementValueFloat();
    bool ParseArrayElementValueString();
    bool ParseFieldName();
    bool ParseFieldType();
    std::optional<std::string> ParseStringLiteral();
    int64_t MnemonicToBuiltinId();
    uint8_t ParseMultiArrayHallmark();

    bool ParseInteger(int64_t *value);
    bool ParseFloat(double *value, bool is64bit);
    bool ParseOperandVreg();
    bool ParseOperandComma();
    bool ParseOperandInteger();
    bool ParseOperandFloat(bool is64bit);
    bool ParseOperandId();
    bool ParseOperandLabel();
    bool ParseOperandField();
    bool ParseOperandType(Type::VerificationType verType);
    bool ParseOperandNone();
    bool ParseOperandString();
    bool ParseOperandLiteralArray();
    bool ParseOperandCall();
    bool ParseOperandSignature(std::string *sign);
    bool ParseOperandSignatureTypesList(std::string *sign);
    bool ParseOperandBuiltinMnemonic();
    bool ParseOperandInitobj();

    void SetFunctionInformation();
    void SetRecordInformation(const std::string &recordName);
    void SetArrayInformation();
    void SetOperationInformation();
    void ParseAsCatchall(const std::vector<Token> &tokens);
    void ParseAsLanguage(const std::vector<Token> &tokens, bool &isLangParsed, bool &isFirstStatement);
    void ParseAsRecord(const std::vector<Token> &tokens);
    void ParseAsArray(const std::vector<Token> &tokens);
    void ParseAsFunction(const std::vector<Token> &tokens);
    void ParseAsUnionField(const std::vector<Token> &tokens);
    void ParseAsBraceRight(const std::vector<Token> &tokens);
    bool ParseAfterLine(bool &isFirstStatement);
    void ParseContextByType(const std::vector<Token> &tokens, bool &isLangParsed, bool &isFirstStatement);
    Expected<Program, Error> ParseAfterMainLoop(const std::string &fileName);
    void ParseResetFunctionLabelsAndParams();
    void ParseResetTables();
    void ParseResetFunctionTable();
    void ParseInsFromFuncTable(ark::pandasm::Function &func);
    void ParseResetRecordTable();
    void ParseResetRecords(const ark::pandasm::Record &record);
    void ParseResetArrayTable();
    void ParseAsLanguageDirective();
    Function::CatchBlock PrepareCatchBlock(bool isCatchall, size_t size, size_t catchallTokensNum,
                                           size_t catchTokensNum);
    void ParseAsCatchDirective();
    void SetError();
    void SetMetadataContextError(const Metadata::Error &err, bool hasValue);

    Expected<char, Error> ParseOctalEscapeSequence(std::string_view s, size_t *i);
    Expected<char, Error> ParseHexEscapeSequence(std::string_view s, size_t *i);
    Expected<char, Error> ParseEscapeSequence(std::string_view s, size_t *i);

    template <class T>
    auto TryEmplaceInTable(bool flag, T &item, const std::string &cid)
    {
        return item.try_emplace(cid, cid, program_.lang, context_.tokens[context_.number - 1].boundLeft,
                                context_.tokens[context_.number - 1].boundRight,
                                context_.tokens[context_.number - 1].wholeLine, flag, lineStric_);
    }

    template <class T>
    bool AddObjectInTable(bool flag, T &item, const std::string &cid = "")
    {
        std::string elem = !cid.empty() ? cid : std::string(context_.GiveToken().data(), context_.GiveToken().length());
        auto [iter, is_inserted] = TryEmplaceInTable(flag, item, elem);

        if (is_inserted) {
            return true;
        }

        if (iter->second.fileLocation->isDefined && flag) {
            return false;
        }

        if (!iter->second.fileLocation->isDefined && flag) {
            iter->second.fileLocation->isDefined = true;
            return true;
        }

        if (!iter->second.fileLocation->isDefined) {
            iter->second.fileLocation->boundLeft = context_.tokens[context_.number - 1].boundLeft;
            iter->second.fileLocation->boundRight = context_.tokens[context_.number - 1].boundRight;
            iter->second.fileLocation->wholeLine = context_.tokens[context_.number - 1].wholeLine;
            iter->second.fileLocation->lineNumber = lineStric_;
        }

        return true;
    }
};

template <>
inline auto Parser::TryEmplaceInTable(bool flag, std::unordered_map<std::string, ark::pandasm::Label> &item,
                                      [[maybe_unused]] const std::string &cid)
{
    return item.try_emplace(std::string(context_.GiveToken().data(), context_.GiveToken().length()),
                            std::string(context_.GiveToken().data(), context_.GiveToken().length()),
                            context_.tokens[context_.number - 1].boundLeft,
                            context_.tokens[context_.number - 1].boundRight,
                            context_.tokens[context_.number - 1].wholeLine, flag, lineStric_);
}

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_PARSER_H
