..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Grammar Summary:

Grammar Summary
###############


.. code-block:: abnf

    literal: Literal;

    identifier: Identifier;

    indexType: 'number';

    type:
        predefinedType
        | typeReference
        | arrayType
        | tupleType
        | functionType
        | unionType
        | literalType
        | keyofType
        | '(' type ')'
        ;

    predefinedType:
        'number' | 'byte' | 'short' | 'int' | 'long' | 'float' | 'double'
        | 'bigint'
        | 'char' | 'boolean'
        | 'object' | 'string' | 'void' | 'never' |'undefined'
        ;

    literalType:
        Literal
        ;

    typeReference:
        typeReferencePart ('.' typeReferencePart)*
        |  identifier '!'
        ;

    typeReferencePart:
        identifier typeArguments?
        ;

    arrayType:
       type '[' ']'
       ;

    functionType:
        '(' ftParameterList? ')' ftReturnType 'throws'?
        ;

    ftParameterList:
        ftParameter (',' ftParameter)* (',' ftRestParameter)?
        | ftRestParameter
        ;

    ftParameter:
        identifier ('?')? ':' type
        ;

    ftRestParameter:
        '...' ftParameter
        ;

    ftReturnType:
        '=>' type
        ;

    tupleType:
        '[' (type (',' type)*)? ']'
        ;

    unionType:
        type ('|' type)*
        ;

    keyofType:
        'keyof' typeReference
        ;

    nullishType:
          type '|' 'null' ('|' 'undefined')?
        | type '|' 'undefined' ('|' 'null')?
        ;

    qualifiedName:
      identifier ('.' identifier )*
      ;

    typeDeclaration:
        classDeclaration
        | interfaceDeclaration
        | enumDeclaration
        | typeAlias
        ;

    typeAlias:
        'type' identifier typeParameters? '=' type
        ;

    variableDeclarations:
        'let' variableDeclarationList
        ;

    variableDeclarationList:
        variableDeclaration (',' variableDeclaration)*
        ;

    variableDeclaration:
        identifier ('?')? ':' ('readonly')? type initializer?
        | identifier initializer
        ;

    initializer:
        '=' expression
        ;

    constantDeclarations:
        'const' constantDeclarationList
        ;

    constantDeclarationList:
        constantDeclaration (',' constantDeclaration)*
        ;

    constantDeclaration:
        identifier (':' type)? initializer
        ;

    functionDeclaration:
        functionOverloadSignature*
        modifiers? 'function' identifier
        typeParameters? signature block?
        ;

    modifiers:
        'native' | 'async'
        ;

    signature:
        parameters returnType? throwMark?
        ;

    parameters:
        '(' parameterList? ')'
        ;

    returnType:
        ':' type
        ;

    throwMark:
        'throws' | 'rethrows'
        ;

    parameterList:
        parameter (',' parameter)* (',' optionalParameters|restParameter)?
        | restParameter
        | optionalParameters
        ;

    parameter:
        identifier ':' 'readonly'? type
        ;

    restParameter:
        '...' parameter
        ;

    optionalParameters:
        optionalParameter (',' optionalParameter)
        ;


    optionalParameter:
        identifier ':' 'readonly'? type '=' expression
        | identifier '?' ':' 'readonly'? type
        ;

    functionOverloadSignature:
      'async'? 'function' identifier typeParameters? signature
      ;

    typeParameters:
        '<' typeParameterList '>'
        ;

    typeParameterList:
        typeParameter (',' typeParameter)*
        ;

    typeParameter:
        ('in' | 'out')? identifier constraint? typeParameterDefault?
        ;

    constraint:
        'extends' typeReference | keyofType | unionType
        ;

    typeParameterDefault:
        '=' typeReference ('[]')?
        ;

    typeArguments:
        '<' typeArgumentList '>'
        ;

    typeArgumentList:
        typeArgument (',' typeArgument)*
        ;

    typeArgument:
        typeReference
        | arrayType
        ;

    expression:
        primaryExpression
        | castExpression
        | instanceOfExpression
        | typeOfExpression
        | nullishCoalescingExpression
        | spreadExpression
        | unaryExpression
        | binaryExpression
        | assignmentExpression
        | conditionalExpression
        | stringInterpolation
        | lambdaExpression
        | dynamicImportExpression
        | launchExpression
        | awaitExpression
        ;

    primaryExpression:
        literal
        | namedReference
        | arrayLiteral
        | objectLiteral
        | thisExpression
        | parenthesizedExpression
        | methodCallExpression
        | fieldAccessExpression
        | indexingExpression
        | functionCallExpression
        | newExpression
        | ensureNotNullishExpression
        ;

    binaryExpression:
        multiplicativeExpression
        | additiveExpression
        | shiftExpression
        | relationalExpression
        | equalityExpression
        | bitwiseAndLogicalExpression
        | conditionalAndExpression
        | conditionalOrExpression
        ;

    objectReference:
        typeReference
        | 'super'
        | primaryExpression
        ;

    arguments:
        '(' argumentSequence? ')'
        ;

    argumentSequence:
        restArgument
        | expression (',' expression)* (',' restArgument)? ','?
        ;

    restArgument:
        '...'? expression
        ;

    namedReference:
      qualifiedName typeArguments?
      ;

    arrayLiteral:
        '[' expressionSequence? ']'
        ;

    expressionSequence:
        expression (',' expression)* ','?
        ;

    objectLiteral:
       '{' valueSequence? '}'
       ;

    valueSequence:
       nameValue (',' nameValue)* ','?
       ;

    nameValue:
       identifier ':' expression
       ;

    recordLiteral:
       '{' keyValueSequence? '}'
       ;

    keyValueSequence:
       keyValue (',' keyValue)* ','?
       ;

    keyValue:
       expression ':' expression
       ;

    spreadExpression:
        '...' expression
        ;

    parenthesizedExpression:
        '(' expression ')'
        ;

    thisExpression:
        'this'
        ;

    fieldAccessExpression:
        objectReference ('.' | '?.') identifier
        ;

    methodCallExpression:
        objectReference ('.' | '?.') identifier typeArguments? arguments block?
        ;

    functionCallExpression:
        expression ('?.' | typeArguments)? arguments block?
        ;

    indexingExpression:
        expression ('?.')? '[' expression ']'
        ;

    newExpression:
        newClassInstance
        | newArrayInstance
        ;

    newClassInstance:
        'new' typeArguments? typeReference arguments?
        ;

    castExpression:
        expression 'as' type
        ;

    instanceOfExpression:
        expression 'instanceof' type
        ;

    typeOfExpression:
        'typeof' expression
        ;

    ensureNotNullishExpression:
        expression '!'
        ;

    nullishCoalescingExpression:
        expression '??' expression
        ;

    unaryExpression:
        expression '++'
        | expression '––'
        | '++' expression
        | '––' expression
        | '+' expression
        | '–' expression
        | '~' expression
        | '!' expression
        ;

    multiplicativeExpression:
        expression '*' expression
        | expression '/' expression
        | expression '%' expression
        ;

    additiveExpression:
        expression '+' expression
        | expression '-' expression
        ;

    shiftExpression:
        expression '<<' expression
        | expression '>>' expression
        | expression '>>>' expression
        ;

    relationalExpression:
        expression '<' expression
        | expression '>' expression
        | expression '<=' expression
        | expression '>=' expression
        ;

    equalityExpression:
        expression ('==' | '===' | '!=' | '!==') expression
        ;

    bitwiseAndLogicalExpression:
        expression '&' expression
        | expression '^' expression
        | expression '|' expression
        ;

    conditionalAndExpression:
        expression '&&' expression
        ;

    conditionalOrExpression:
        expression '||' expression
        ;

    assignmentExpression:
        lhsExpression assignmentOperator rhsExpression
        ;

    assignmentOperator
        : '='
        | '+='  | '-='  | '*='   | '='  | '%='
        | '<<=' | '>>=' | '>>>='
        | '&='  | '|='  | '^='
        ;

    lhsExpression:
        expression
        ;

    rhsExpression:
        expression
        ;

    conditionalExpression:
        expression '?' expression ':' expression
        ;

    stringInterpolation:
        '`' (BacktickCharacter | embeddedExpression)* '`'
        ;

    embeddedExpression:
        '${' expression '}'
        ;

    lambdaExpression:
        ('async'|typeParameters)? lambdaSignature '=>' lambdaBody
        ;

    lambdaBody:
        expression | block
        ;

    lambdaSignature:
        lambdaParameters returnType? throwMark?
        ;

    lambdaParameters:
        '(' lambdaParameterList? ')'
        | identifier
        ;

    lambdaParameterList:
        lambdaParameter (',' lambdaParameter)*
               (',' lambdaOptionalParameters|lambdaRestParameter)? 
        | lambdaRestParameter
        | optionalParameters
        ;

    lambdaParameter:
        identifier (':' 'readonly'? type)?
        ;

    lambdaRestParameter:
        '...' lambdaParameter
        ;

    lambdaOptionalParameters:
        lambdaOptionalParameter (',' lambdaOptionalParameter)
        ;
    
    lambdaOptionalParameter:
        identifier '?' (':' 'readonly'? type)?
        ;

    dynamicImportExpression:
        'import' '(' expression ')'
        ;

    constantExpression:
        expression
        ;

    statement:
        expressionStatement
        | block
        | localDeclaration
        | ifStatement
        | loopStatement
        | breakStatement
        | continueStatement
        | returnStatement
        | switchStatement
        | throwStatement
        | tryStatement
        ;

    expressionStatement:
        expression
        ;

    block:
        '{' statement* '}'
        ;

    localDeclaration:
        variableDeclaration
        | constantDeclaration
        | typeDeclaration
        ;

    ifStatement:
        'if' '(' expression ')' thenStatement
        ('else' elseStatement)?
        ;

    thenStatement:
        statement
        ;

    elseStatement:
        statement
        ;

    loopStatement:
        (identifier ':')?
        whileStatement
        | doStatement
        | forStatement
        | forOfStatement
        ;

    whileStatement:
        'while' '(' expression ')' statement
        ;

    doStatement
        : 'do' statement 'while' '(' expression ')'
        ;

    forStatement:
        'for' '(' forInit? ';' expression? ';' forUpdate? ')' statement
        ;

    forInit:
        expressionSequence
        | variableDeclarations
        ;

    forUpdate:
        expressionSequence
        ;

    forOfStatement:
        'for' '(' forVariable 'of' expression ')' statement
        ;

    forVariable:
        identifier | ('let' | 'const') identifier (':' type)?
        ;

    breakStatement:
        'break' identifier?
        ;

    continueStatement:
        'continue' identifier?
        ;

    returnStatement:
        'return' expression?
        ;

    switchStatement:
        (identifier ':')? 'switch' '(' expression ')' switchBlock
        ;

    switchBlock
        : '{' caseClause* defaultClause? caseClause* '}'
        ;

    caseClause
        : 'case' expression ':' statement*
        ;

    defaultClause
        : 'default' ':' statement*
        ;

    throwStatement:
        'throw' expression
        ;

    tryStatement:
          'try' block catchClauses finallyClause?
          ;

    catchClauses:
          typedCatchClause* catchClause?
          ;

    catchClause:
          'catch' '(' identifier ')' block
          ;

    typedCatchClause:
          'catch' '(' identifier ':' typeReference ')' block
          ;

    finallyClause:
          'finally' block
          ;


    classDeclaration:
        classModifier? 'class' identifier typeParameters?
          classExtendsClause? implementsClause? classBody
        ;

    classModifier:
        'abstract' | 'final'
        ;

    classExtendsClause:
        'extends' typeReference
        ;

    implementsClause:
        'implements' interfaceTypeList
        ;

    interfaceTypeList:
        typeReference (',' typeReference)*
        ;

    classBody:
        '{'
           classBodyDeclaration* classInitializer? classBodyDeclaration*
        '}'
        ;

    classBodyDeclaration:
        accessModifier?
        ( constructorDeclaration
        | classFieldDeclaration
        | classMethodDeclaration
        | classAccessorDeclaration
        )
        ;

    accessModifier:
        'private'
        | 'internal'
        | 'protected'
        | 'public'
        ;

    classFieldDeclaration:
        fieldModifier* variableDeclaration
        ;

    fieldModifier:
        'static' | 'readonly'
        ;

    classMethodDeclaration:
        methodOverloadSignature*
        methodModifier* typeParameters? identifier signature block?
        ;

    methodModifier:
        'abstract'
        | 'static'
        | 'final'
        | 'override'
        | 'native'
        | 'async'
        ;

    methodOverloadSignature:
        methodModifier* identifier signature
        ;

    classAccessorDeclaration:
        accessorModifier*
        ( 'get' identifier '(' ')' returnType block?
        | 'set' identifier '(' parameter ')' block?
        )
        ;

    accessorModifier:
        'abstract'
        | 'static'
        | 'final'
        | 'override'
        ;

    classInitializer
        : 'static' block
        ;

    constructorDeclaration:
        constructorOverloadSignature*
        'constructor' parameters throwMark? constructorBody
        ;

    constructorOverloadSignature:
        accessModifier? 'constructor' signature
        ;

    constructorBody:
        '{' statement* constructorCall? statement* '}'
        ;

    constructorCall:
        'this' arguments
        | 'super' arguments
        ;

    interfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause? '{' interfaceMember* '}'
        ;

    interfaceExtendsClause:
        'extends' interfaceTypeList
        ;


    interfaceMember
        : interfaceProperty
        | interfaceMethodDeclaration
        ;

    interfaceProperty:
        'readonly'? identifier '?'? ':' type
        | 'get' identifier '(' ')' returnType
        | 'set' identifier '(' parameter ')'
        ;

    interfaceMethodDeclaration:
        identifier signature
        | interfaceDefaultMethodDeclaration
        ;

    enumDeclaration:
        'const'? 'enum' identifier '{' enumConstantList '}'
        ;

    enumConstantList:
        enumConstant (',' enumConstant)* ','?
        ;

    enumConstant:
        identifier ('=' constantExpression)?
        ;

    compilationUnit:
        separateModuleDeclaration
        | packageDeclaration
        | declarationModule
        ;

    packageDeclaration:
        packageModule+
        ;

    separateModuleDeclaration:
        importDirective* (topDeclaration | topLevelStatements | exportDirective)*
        ;

    importDirective:
        'import'
        (allBinding|selectiveBindigns|defaultBinding|typeBinding 'from')?
        importPath
        ;

    allBinding:
        '*' bindingAlias
        ;

    selectiveBindigns:
        '{' nameBinding (',' nameBinding)* '}'
        ;

    defaultBinding:
        identifier | ( '{' 'default' 'as' identifier '}' )
        ;

    typeBinding:
        'type' selectiveBindigns
        ;

    nameBinding:
        qualifiedName bindingAlias?
        ;

    bindingAlias:
        'as' identifier
        ;

    importPath:
        StringLiteral
        ;

    declarationModule:
        importDirective*
        ( 'export'? ambientDeclaration
        | 'export'? typeAlias
        | selectiveExportDirective
        )*
        ;

    topDeclaration:
        ('export' 'default'?)?
        ( typeDeclaration
        | variableDeclarations
        | constantDeclarations
        | functionDeclaration
        | extensionFunctionDeclaration
        )
        ;

    exportDirective:
        selectiveExportDirective
        | singleExportDirective
        | exportTypeDirective
        | reExportDirective
        ;

    selectiveExportDirective:
        'export' selectiveBindigns
        ;

    singleExportDirective:
        'export' identifier
        ;

    exportTypeDirective:
        'export' 'type' selectiveBindigns
        ;

    reExportDirective:
        'export' ('*' | selectiveBindigns) 'from' importPath
        ;

    topLevelStatements:
        statement*
        ;

    ambientDeclaration:
        'declare'
        ( ambientConstantDeclaration
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | 'const'? enumDeclaration
        )
        ;

    ambientConstantDeclaration:
        'const' ambientConstList ';'
        ;

    ambientConstList:
        ambientConst (',' ambientConst)*
        ;

    ambientConst:
        identifier (':' type)? initializer
        ;

    ambientFunctionDeclaration:
        ambientFunctionOverloadSignature*
        'function' identifier
        typeParameters? signature
        ;

    ambientFunctionOverloadSignature:
        'declare'? 'function' identifier
          typeParameters? signature ';'
        ;

    ambientClassDeclaration:
        'class' identifier typeParameters?
        classExtendsClause? implementsClause?
        '{' ambientClassBodyDeclaration* '}'
        ;

    ambientClassBodyDeclaration:
        ambientAccessModifier?
        ( ambientFieldDeclaration
        | ambientConstructorDeclaration
        | ambientMethodDeclaration
        | ambientAccessorDeclaration
        | ambientIndexerDeclaration
        | ambientCallSignatureDeclaration
        | ambientIterableDeclaration
        )
        ;


    ambientAccessModifier:
        'public' | 'protected'
        ;

    ambientFieldDeclaration:
        ambientFieldModifier* identifier ':' type
        ;

    ambientFieldModifier:
        'static' | 'readonly'
        ;

    ambientConstructorDeclaration:
        'constructor' parameters throwMark?
        ;

    ambientMethodDeclaration:
        ambientMethodOverloadSignature*
        ambientMethodModifier* identifier signature
        ;

    ambientMethodOverloadSignature:
        ambientMethodModifier* identifier signature ';'
        ;


    ambientMethodModifier:
        'static'
        ;

    ambientAccessorDeclaration:
        ambientMethodModifier*
        ( 'get' identifier '(' ')' returnType
        | 'set' identifier '(' parameter ')'
        )
        ;

    ambientIndexerDeclaration:
        'readonly'? '[' identifier ':' indexType ']' returnType
        ;

    ambientCallSignatureDeclaration:
        signature
        ;

    ambientIterableDeclaration:
        '[Symbol.iterator]' '(' ')' returnType
        ;

    ambientInterfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause?
        '{' ambientInterfaceMember* '}'
        ;

    ambientInterfaceMember
        : interfaceProperty
        | interfaceMethodDeclaration
        | ambientIndexerDeclaration
        | ambientCallSignatureDeclaration
        | ambientIterableDeclaration
        ;

    ambientNamespaceDeclaration:
        'namespace' identifier '{' ambientNamespaceElement* '}'
        ;

    ambientNamespaceElement:
        ambientNamespaceElementDeclaration | selectiveExportDirective
    ;

    ambientNamespaceElementDeclaration:
        'export'?
        ( ambientConstantDeclaration
        | ambientFunctionDeclaration
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | 'const'? enumDeclaration
        | typeAlias
        )
        ;



      newArrayInstance:
          'new' arrayElementType dimensionExpression+ (arrayElement)?
          ;

      arrayElementType:
          typeReference
          | '(' type ')'
          ;

      dimensionExpression:
          '[' expression ']'
          ;

      arrayElement:
          '(' expression ')'
          ;

    interfaceDefaultMethodDeclaration:
        'private'? identifier signature block
        ;

    extensionFunctionDeclaration:
        'static'? 'function' typeParameters? typeReference '.' identifier
        signature block
        ;

    trailingLambdaCall:
        ( objectReference '.' identifier typeArguments?
        | expression ('?.' | typeArguments)?
        )
        arguments block
        ;

      launchExpression:
        'launch' functionCallExpression|methodCallExpression|lambdaExpression;

      awaitExpression:
        'await' expression
        ;


      packageModule:
          packageHeader packageModuleDeclaration
          ;

      packageHeader:
          'package' qualifiedName
          ;

      packageModuleDeclaration:
          importDirective* packageTopDeclaration*
          ;

      packageTopDeclaration:
          topDeclaration | packageInitializer
          ;

      packageInitializer:
          'static' block
          ;

    annotationDeclaration:
        '@interface' identifier '{' annotationField* '}'
        ;

    annotationField:
        identifier ':' type constInitializer?
        ;

    constInitializer:
        '=' constantExpression
        ;

    annotationUsage:
        '@' qualifiedName annotationValues?
        ;

    annotationValues:
        '(' (objectLiteral | constantExpression)? ')'
        ;

    ambientAnnotationDeclaration:
        'declare' annotationDeclaration
        ;

    Identifier:
      IdentifierStart IdentifierPart*
      ;

    IdentifierStart:
      UnicodeIDStart
      | '$'
      | '_'
      | '\\' EscapeSequence
      ;

    IdentifierPart:
      UnicodeIDContinue
      | '$'
      | ZWNJ
      | ZWJ
      | '\\' EscapeSequence
      ;

    ZWJ:
     '\u200C'
    ;

    ZWNJ:
     '\u200D'
    ;

    UnicodeIDStart
      : Letter
      | ['$']
      | '\\' UnicodeEscapeSequence;

    UnicodeIDContinue
      : UnicodeIDStart
      | UnicodeDigit
      | '\u200C'
      | '\u200D';

    UnicodeEscapeSequence:
      'u' HexDigit HexDigit HexDigit HexDigit
      | 'u' '{' HexDigit HexDigit+ '}'
      ;

    Letter
      : UNICODE_CLASS_LU
      | UNICODE_CLASS_LL
      | UNICODE_CLASS_LT
      | UNICODE_CLASS_LM
      | UNICODE_CLASS_LO
      ;

    UnicodeDigit
      : UNICODE_CLASS_ND
      ;

    Literal:
      IntegerLiteral
      | FloatLiteral
      | BigIntLiteral
      | BooleanLiteral
      | StringLiteral
      | MultilineStringLiteral
      | NullLiteral
      | UndefinedLiteral
      | CharLiteral
      ;

    IntegerLiteral:
      DecimalIntegerLiteral
      | HexIntegerLiteral
      | OctalIntegerLiteral
      | BinaryIntegerLiteral
      ;

    DecimalIntegerLiteral:
      '0'
      | DecimalDigitNotNull ('_'? DecimalDigit)*
      ;

    DecimalDigit:
      [0-9]
      ;

    DecimalDigitNotNull:
      [1-9]
      ;

    HexIntegerLiteral:
      '0' [xX]  ( HexDigit
      | HexDigit (HexDigit | '_')* HexDigit
      )
      ;

    HexDigit:
      [0-9a-fA-F]
      ;

    OctalIntegerLiteral:
      '0' [oO] ( OctalDigit
      | OctalDigit (OctalDigit | '_')* OctalDigit )
      ;

    OctalDigit:
      [0-7]
      ;

    BinaryIntegerLiteral:
      '0' [bB] ( BinaryDigit
      | BinaryDigit (BinaryDigit | '_')* BinaryDigit )
      ;

    BinaryDigit:
      [0-1]
      ;

    FloatLiteral:
        DecimalIntegerLiteral '.' FractionalPart? ExponentPart? FloatTypeSuffix?
        | '.' FractionalPart ExponentPart? FloatTypeSuffix?
        | DecimalIntegerLiteral ExponentPart FloatTypeSuffix?
        ;

    ExponentPart:
        [eE] [+-]? DecimalIntegerLiteral
        ;

    FractionalPart:
        DecimalDigit
        | DecimalDigit (DecimalDigit | '_')* DecimalDigit
        ;

    FloatTypeSuffix:
        'f'
        ;

    BigIntLiteral:
      '0n'
      | [1-9] ('_'? [0-9])* 'n'
      ;

    BooleanLiteral:
        'true' | 'false'
        ;

    StringLiteral:
        '"' DoubleQuoteCharacter* '"'
        | '\'' SingleQuoteCharacter* '\''
        ;

    DoubleQuoteCharacter:
        ~["\\\r\n]
        | '\\' EscapeSequence
        ;

    SingleQuoteCharacter:
        ~['\\\r\n]
        | '\\' EscapeSequence
        ;

    EscapeSequence:
        ['"bfnrtv0\\]
        | 'x' HexDigit HexDigit
        | 'u' HexDigit HexDigit HexDigit HexDigit
        | 'u' '{' HexDigit+ '}'
        | ~[1-9xu\r\n]
        ;

    MultilineStringLiteral:
        '`' (BacktickCharacter)* '`'
        ;

    BacktickCharacter:
        ~['\\\r\n]
        | '\\' EscapeSequence
        | LineContinuation
        ;

     LineContinuation:
        '\\' [\r\n\u2028\u2029]+
        ;

    NullLiteral:
        'null'
        ;

    UndefinedLiteral:
        'undefined'
        ;

    CharLiteral:
        'c\'' SingleQuoteCharacter '\''
        ;

|


.. raw:: pdf

   PageBreak





