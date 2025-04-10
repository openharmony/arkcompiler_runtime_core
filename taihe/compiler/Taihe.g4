/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
grammar Taihe;

/////////////
// Grammar //
/////////////

spec
    : (UseLst_uses += use | SpecFieldLst_fields += specField | ScopeAttrLst_inner_attrs += scopeAttr)*
      EOF
    ;

scopeAttr
    : AT EXCLAMATION TOKEN_name = ID (LEFT_PARENTHESIS (AttrValLst_vals += attrVal (COMMA AttrValLst_vals += attrVal)* COMMA?)? RIGHT_PARENTHESIS)?
    ;

declAttr
    : AT TOKEN_name = ID (LEFT_PARENTHESIS (AttrValLst_vals += attrVal (COMMA AttrValLst_vals += attrVal)* COMMA?)? RIGHT_PARENTHESIS)?
    ;

use
    : KW_USE PkgName_pkg_name = pkgName (KW_AS TOKENOpt_pkg_alias = ID)? SEMICOLON # usePackage
    | KW_FROM PkgName_pkg_name = pkgName KW_USE DeclAliasPairLst_decl_alias_pairs += declAliasPair (COMMA DeclAliasPairLst_decl_alias_pairs += declAliasPair)* SEMICOLON # useSymbol
    ;

pkgName
    : (TOKENLst_parts += ID DOT)* TOKENLst_parts += ID
    ;

declAliasPair
    : TOKEN_decl_name = ID (KW_AS TOKENOpt_decl_alias = ID)?
    ;

specField
    : (DeclAttrLst_forward_attrs += declAttr)*
      KW_ENUM TOKEN_name = ID (COLON TypeOpt_enum_ty = type)?
      LEFT_BRACE (EnumItemLst_fields += enumItem (COMMA EnumItemLst_fields += enumItem)* COMMA?)? RIGHT_BRACE # enum
    | (DeclAttrLst_forward_attrs += declAttr)*
      KW_STRUCT TOKEN_name = ID
      LEFT_BRACE (StructFieldLst_fields += structField | ScopeAttrLst_inner_attrs += scopeAttr)* RIGHT_BRACE # struct
    | (DeclAttrLst_forward_attrs += declAttr)*
      KW_UNION TOKEN_name = ID
      LEFT_BRACE (UnionFieldLst_fields += unionField | ScopeAttrLst_inner_attrs += scopeAttr)* RIGHT_BRACE # union
    | (DeclAttrLst_forward_attrs += declAttr)*
      KW_INTERFACE TOKEN_name = ID
      (COLON InterfaceParentLst_extends += interfaceParent (COMMA InterfaceParentLst_extends += interfaceParent)* COMMA?)?
      LEFT_BRACE (InterfaceFieldLst_fields += interfaceField | ScopeAttrLst_inner_attrs += scopeAttr)* RIGHT_BRACE # interface
    | (DeclAttrLst_forward_attrs += declAttr)*
      KW_FUNCTION TOKEN_name = ID
      LEFT_PARENTHESIS (ParameterLst_parameters += parameter (COMMA ParameterLst_parameters += parameter)* COMMA?)? RIGHT_PARENTHESIS (COLON (TypeOpt_return_ty = type | KW_VOID))? SEMICOLON # globalFunction
    ;

enumItem
    : (DeclAttrLst_forward_attrs += declAttr)*
      TOKEN_name = ID (ASSIGN_TO AttrValOpt_val = attrVal)? # enumProperty
    ;

structField
    : (DeclAttrLst_forward_attrs += declAttr)*
      TOKEN_name = ID
      COLON Type_ty = type SEMICOLON # structProperty
    ;

unionField
    : (DeclAttrLst_forward_attrs += declAttr)*
      TOKEN_name = ID
      (COLON TypeOpt_ty = type)? SEMICOLON # unionProperty
    ;

interfaceField
    : (DeclAttrLst_forward_attrs += declAttr)*
      TOKEN_name = ID
      LEFT_PARENTHESIS (ParameterLst_parameters += parameter (COMMA ParameterLst_parameters += parameter)* COMMA?)? RIGHT_PARENTHESIS (COLON (TypeOpt_return_ty = type | KW_VOID))? SEMICOLON # interfaceFunction
    ;

interfaceParent
    : (DeclAttrLst_forward_attrs += declAttr)*
      Type_ty = type
    ;

parameter
    : (DeclAttrLst_forward_attrs += declAttr)*
      TOKEN_name = ID COLON Type_ty = type
    ;

///////////////
// Attribute //
///////////////

attrItem
    : TOKEN_name = ID (ASSIGN_TO AttrValOpt_val = attrVal)? # simpleAttrItem
    | TOKEN_name = ID LEFT_PARENTHESIS (AttrValLst_vals += attrVal (COMMA AttrValLst_vals += attrVal)* COMMA?)? RIGHT_PARENTHESIS # tupleAttrItem
    ;

attrVal
    : StringExpr_expr = stringExpr # stringAttrVal
    | IntExpr_expr = intExpr # intAttrVal
    | FloatExpr_expr = floatExpr # floatAttrVal
    | BoolExpr_expr = boolExpr # boolAttrVal
    ;

//////////
// Type //
//////////

type
    : PkgName_pkg_name = pkgName DOT TOKEN_decl_name = ID # longType
    | TOKEN_decl_name = ID # shortType
    | TOKEN_decl_name = ID LESS_THAN (TypeLst_args += type (COMMA TypeLst_args += type)* COMMA?)? GREATER_THAN # genericType
    | <assoc = right>
      LEFT_PARENTHESIS (ParameterLst_parameters += parameter (COMMA ParameterLst_parameters += parameter)* COMMA?)? RIGHT_PARENTHESIS ARROW (TypeOpt_return_ty = type | KW_VOID) # callbackType
    ;

////////////////
// Expression //
////////////////

floatExpr
    : TOKEN_val = FLOAT_LITERAL # literalFloatExpr
    | LEFT_PARENTHESIS FloatExpr_expr = floatExpr RIGHT_PARENTHESIS # parenthesisFloatExpr
    | TOKEN_op = (PLUS | MINUS | TILDE) FloatExpr_expr = floatExpr # unaryFloatExpr
    | FloatExpr_left = floatExpr TOKEN_op = (STAR | SLASH) FloatExpr_right = floatExpr # binaryFloatExpr
    | FloatExpr_left = floatExpr TOKEN_op = (PLUS | MINUS) FloatExpr_right = floatExpr # binaryFloatExpr
    | KW_IF BoolExpr_cond = boolExpr KW_THEN FloatExpr_then_expr = floatExpr KW_ELSE FloatExpr_else_expr = floatExpr # conditionalFloatExpr
    ;

intExpr
    : TOKEN_val = (DEC_LITERAL | OCT_LITERAL | HEX_LITERAL | BIN_LITERAL) # literalIntExpr
    | LEFT_PARENTHESIS IntExpr_expr = intExpr RIGHT_PARENTHESIS # parenthesisIntExpr
    | TOKEN_op = (PLUS | MINUS | TILDE) IntExpr_expr = intExpr # unaryIntExpr
    | IntExpr_left = intExpr TOKEN_op = (STAR | SLASH | PERCENT) IntExpr_right = intExpr # binaryIntExpr
    | IntExpr_left = intExpr TOKEN_op = (PLUS | MINUS) IntExpr_right = intExpr # binaryIntExpr
    | IntExpr_left = intExpr (TOKEN_ch = LESS_THAN LESS_THAN | TOKEN_ch = GREATER_THAN GREATER_THAN) IntExpr_right = intExpr # binaryIntShiftExpr
    | IntExpr_left = intExpr TOKEN_op = AMPERSAND IntExpr_right = intExpr # binaryIntExpr
    | IntExpr_left = intExpr TOKEN_op = CARET IntExpr_right = intExpr # binaryIntExpr
    | IntExpr_left = intExpr TOKEN_op = PIPE IntExpr_right = intExpr # binaryIntExpr
    | KW_IF BoolExpr_cond = boolExpr KW_THEN IntExpr_then_expr = intExpr KW_ELSE IntExpr_else_expr = intExpr # conditionalIntExpr
    ;

boolExpr
    : TOKEN_val = (KW_TRUE | KW_FALSE) # literalBoolExpr
    | LEFT_PARENTHESIS BoolExpr_expr = boolExpr RIGHT_PARENTHESIS # parenthesisBoolExpr
    | TOKEN_op = EXCLAMATION BoolExpr_expr = boolExpr # unaryBoolExpr
    | IntExpr_left = intExpr TOKEN_op = (EQUAL_TO | NOT_EQUAL_TO | LESS_EQUAL_TO | GREATER_EQUAL_TO | LESS_THAN | GREATER_THAN) IntExpr_right = intExpr # intComparisonBoolExpr
    | FloatExpr_left = floatExpr TOKEN_op = (EQUAL_TO | NOT_EQUAL_TO | LESS_EQUAL_TO | GREATER_EQUAL_TO | LESS_THAN | GREATER_THAN) FloatExpr_right = floatExpr # floatComparisonBoolExpr
    | BoolExpr_left = boolExpr TOKEN_op = (AND | OR) BoolExpr_right = boolExpr # binaryBoolExpr
    | KW_IF BoolExpr_cond = boolExpr KW_THEN BoolExpr_then_expr = boolExpr KW_ELSE BoolExpr_else_expr = boolExpr # conditionalBoolExpr
    ;

stringExpr
    : TOKENLst_vals += STRING_LITERAL+ # literalStringExpr
    | LEFT_PARENTHESIS StringExpr_expr = stringExpr RIGHT_PARENTHESIS # parenthesisStringExpr
    ;

///////////
// Token //
///////////

SEMICOLON
    : ';'
    ;

COLON
    : ':'
    ;

DOT
    : '.'
    ;

ARROW
    : '=>'
    ;

COMMA
    : ','
    ;

LEFT_BRACE
    : '{'
    ;

RIGHT_BRACE
    : '}'
    ;

LEFT_PARENTHESIS
    : '('
    ;

RIGHT_PARENTHESIS
    : ')'
    ;

LEFT_BRACKET
    : '['
    ;

RIGHT_BRACKET
    : ']'
    ;

ASSIGN_TO
    : '='
    ;

PLUS
    : '+'
    ;

MINUS
    : '-'
    ;

STAR
    : '*'
    ;

SLASH
    : '/'
    ;

PERCENT
    : '%'
    ;

AT
    : '@'
    ;

DOLLAR
    : '$'
    ;

QUESTION_MARK
    : '?'
    ;

TILDE
    : '~'
    ;

AMPERSAND
    : '&'
    ;

PIPE
    : '|'
    ;

CARET
    : '^'
    ;

EXCLAMATION
    : '!'
    ;

AND
    : '&&'
    ;

OR
    : '||'
    ;

LESS_THAN
    : '<'
    ;

GREATER_THAN
    : '>'
    ;

LESS_EQUAL_TO
    : '<='
    ;

GREATER_EQUAL_TO
    : '>='
    ;

EQUAL_TO
    : '=='
    ;

NOT_EQUAL_TO
    : '!='
    ;

KW_IF
    : 'if'
    ;

KW_THEN
    : 'then'
    ;

KW_ELSE
    : 'else'
    ;

KW_USE
    : 'use'
    ;

KW_AS
    : 'as'
    ;

KW_FROM
    : 'from'
    ;

KW_ENUM
    : 'enum'
    ;

KW_UNION
    : 'union'
    ;

KW_STRUCT
    : 'struct'
    ;

KW_INTERFACE
    : 'interface'
    ;

KW_FUNCTION
    : 'function'
    ;

KW_TRUE
    : 'TRUE'
    ;

KW_FALSE
    : 'FALSE'
    ;

KW_VOID
    : 'void'
    ;

STRING_LITERAL
    : '"' (ESCAPE_SEQUENCE | ~ ('\\' | '"'))* '"'
    | '"""' .*? '"""'
    ;

fragment ESCAPE_SEQUENCE
    : '\\' ('b' | 't' | 'n' | 'f' | 'r' | '"' | '\'' | '\\')
    | UNICODE_ESCAPE
    | OCTAL_ESCAPE
    ;

fragment OCTAL_ESCAPE
    : '\\' 'x' HEX_DIGIT HEX_DIGIT
    ;

fragment UNICODE_ESCAPE
    : '\\' 'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT
    ;

fragment HEX_DIGIT
    : '0' .. '9'
    | 'a' .. 'f'
    | 'A' .. 'F'
    ;

fragment OCT_DIGIT
    : '0' .. '7'
    ;

fragment BIN_DIGIT
    : '0'
    | '1'
    ;

DEC_LITERAL
    : DIGIT+
    ;

HEX_LITERAL
    : '0x' HEX_DIGIT+
    ;

OCT_LITERAL
    : '0o' OCT_DIGIT+
    ;

BIN_LITERAL
    : '0b' HEX_DIGIT+
    ;

FLOAT_LITERAL
    : DIGIT* '.' DIGIT*
    ;

ID
    : (UNDERLINE | LETTER) (UNDERLINE | LETTER | DIGIT)*
    ;

fragment LETTER
    : 'A' .. 'Z'
    | 'a' .. 'z'
    ;

fragment UNDERLINE
    : '_'
    ;

fragment DIGIT
    : '0' .. '9'
    ;

WS
    : (' ' | '\r' | '\t' | '\u000C' | '\n') -> channel (HIDDEN)
    ;

MULTI_LINE_COMMENT
    : '/*' .*? '*/' -> channel (HIDDEN)
    ;

SINGLE_LINE_COMMENT
    : '//' ~ ('\n' | '\r')* '\r'? '\n'? -> channel (HIDDEN)
    ;

ERROR_CHAR
    : .
    ;
