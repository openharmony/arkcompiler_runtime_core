#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import os
import re
import sys
import traceback
from clade import Clade
from clang.cindex import Index
from clang.cindex import TokenKind
from clang.cindex import CursorKind
from clang.cindex import TranslationUnitLoadError
import yamale
import yaml

BINDINGS_FILE_NAME = "bindings.yml"

# File describing structure of bindings.yml files and other restrictions on their content
# This file is used to validate bindings.yml with yamale module
BINDINGS_SCHEMA_FILE = '%s/%s' % (os.path.dirname(os.path.realpath(__file__)), "bindings_schema.yml")

CPP_CODE_TO_GET_TEST_ID_IN_PANDA = "IntrusiveTest::GetId()"
CPP_HEADER_COMPILE_OPTION_LIST = ["-x", "c++-header"]
CLANG_CUR_WORKING_DIR_OPT = "-working-directory"
HEADER_FILE_EXTENSION = ".h"


# Parser of arguments and options
class ArgsParser:

    def __init__(self):
        parser = argparse.ArgumentParser(description="Source code instrumentation for intrusive testing")
        parser.add_argument(
            'src_dir',
            metavar='SRC_DIR',
            nargs=1,
            type=str,
            help="Directory with source code files for instrumentation")

        parser.add_argument(
            'clade_file',
            metavar='CLADE_FILE',
            nargs=1,
            type=str,
            help="File with build commands intercepted by 'clade' tool")

        parser.add_argument(
            'test_suite_dir',
            metavar='TEST_SUITE_DIR',
            nargs=1,
            type=str,
            help="Directory of intrusive test suite")

        self.__args = parser.parse_args()
        self.__srcDir = None
        self.__cladeFile = None
        self.__testSuiteDir = None

    # raise: FatalError
    def getSrcDir(self):
        if(self.__srcDir is None):
            self.__srcDir = os.path.abspath(self.__args.src_dir[0])
            if(not os.path.isdir(self.__srcDir)):
                errMsg = "%s argument %s is not an existing directory: %s"\
                    %("1st", "SRC_DIR", self.__args.src_dir[0])
                raise FatalError(errMsg)
        return self.__srcDir

    # raise: FatalError
    def getCladeFile(self):
        if(self.__cladeFile is None):
            self.__cladeFile = os.path.abspath(self.__args.clade_file[0])
            if(not os.path.isfile(self.__cladeFile)):
                errMsg = "%s argument %s is not an existing file: %s"\
                    %("2nd", "CLADE_FIlE", self.__args.clade_file[0])
                raise FatalError(errMsg)
        return self.__cladeFile

    # raise: FatalError
    def getTestSuiteDir(self):
        if(self.__testSuiteDir is None):
            self.__testSuiteDir = os.path.abspath(self.__args.test_suite_dir[0])
            if(not os.path.isdir(self.__testSuiteDir)):
                errMsg = "%s argument %s is not an existing directory: %s"\
                        %("3d", "TEST_SUITE_DIR", self.__args.test_suite_dir[0])
                raise FatalError(errMsg)
        return self.__testSuiteDir


# User defined exception class
class FatalError(Exception):

    def __init__(self, msg, *args):
        super().__init__(args)
        self.__msg = msg

    def getErrMsg(self):
        return self.__msg

    def __str__(self):
        return self.getErrMsg()


# Class for auxillary functions
class Util:

    # raise: OSError
    @staticmethod
    def findFilesByName(directory, fileName, fileSet):
        for name in os.listdir(directory):
            path = os.path.join(directory, name)
            if(os.path.isfile(path) and (name == fileName)):
                fileSet.add(path)
            elif(os.path.isdir(path) and (name != "..") and (name != ".")):
                Util.findFilesByName(path, fileName, fileSet)

    # raise: OSError
    @staticmethod
    def findFilesByExtension(directory, extension, isRecursive, fileSet):
        for name in os.listdir(directory):
            path = os.path.join(directory, name)
            if(os.path.isfile(path)):
                root, ext = os.path.splitext(path)
                if(ext == extension):
                    fileSet.add(path)
            elif(isRecursive \
                and os.path.isdir(path) \
                and (name != "..") \
                and (name != ".")):
                Util.findFilesByExtension(
                    path,
                    extension,
                    isRecursive,
                    fileSet)

    # raise: OSError
    @staticmethod
    def readFileLines(filePath):
        fd = open(filePath, "r")
        try:
            lines = fd.readlines()
        finally:
            fd.close()
        return lines

    # raise: OSError
    @staticmethod
    def writeLinesToFile(filePath, lines):
        fd = os.open(filePath, os.O_RDWR, 0o777)
        fo = os.fdopen(fd, "w+")
        try:
            fo.writelines(lines)
        finally:
            fo.close()

    @staticmethod
    def blankStringToNone(s):
        if(s is not None):
            match = re.match(r'^\s*$', s)
            if(match):
                return None
        return s


# Class describing location of a synchronization point
class SyncPoint:

    def __init__(self, file, index, cl=None, method=None, source=None):
        self._file = file
        self._index = index
        # In case the key is defined but with empty value, we set 'None' constant to encode that the key is not provided
        self._cl = Util.blankStringToNone(cl)
        self._method = Util.blankStringToNone(method)
        self._source = Util.blankStringToNone(source)

    @property
    def file(self):
        return self._file

    @property
    def index(self):
        return self._index

    @property
    def cl(self):
        return self._cl

    @property
    def method(self):
        return self._method

    @property
    def source(self):
        return self._source

    def hasClass(self):
        return self._cl is not None

    def hasMethod(self):
        return self._method is not None

    def hasSource(self):
        return self._source is not None

    def __eq__(self, other): 
        if not isinstance(other, SyncPoint):
            return False
        return (self.file == other.file) \
            and (self.index == other.index) \
            and (self.cl == other.cl) \
            and (self.method == other.method) \
            and (self.source == other.source)

    def __hash__(self):
        return hash((self.file, self.index, self.cl, self.method, self.source))

    def toString(self):
        s = ["file: %s"%(self.file), "index: %s"%(str(self.index))]
        if(self.hasClass()):
            s.append("class: %s"%(str(self.cl)))
        if(self.hasMethod()):
            s.append("method: %s"%(str(self.method)))
        if(self.hasSource()):
            s.append("source: %s"%(str(self.source)))
        return ", ".join(s)


# Class describing a synchronization action (code)
# of a single test at one synchronization point
class SyncAction:

    ID = 0

    def __init__(self, code):
        self._code = code
        self._id = SyncAction.ID
        SyncAction.ID += 1

    @property
    def code(self):
        return self._code

    @property
    def id(self):
        return self._id

    def __eq__(self, other):
        if not isinstance(other, SyncAction):
            return False
        return self._id == other.id

    def __hash__(self):
        return hash(self._id)


# Abstraction for a source code file
class SourceFile:

    def __init__(self, path):
        self._path = path

    @property
    def path(self):
        return self._path

    def __eq__(self, other): 
        if not isinstance(other, SourceFile):
            return False
        return self.path == other.path

    def __hash__(self):
        return hash(self._path)


# Abstraction for a header file, i.e.
# file included by another source or header file
class HeaderFile(SourceFile):
    def __init__(self, path, includingSourceFilePath):
        self._includingSourceFilePath = includingSourceFilePath
        super().__init__(path)

    @property
    def includingSourceFilePath(self):
        return self._includingSourceFilePath

    def __eq__(self, other): 
        if not isinstance(other, HeaderFile):
            return False
        return super().__eq__(other) \
            and (self.includingSourceFilePath == other.includingSourceFilePath)

    def __hash__(self):
        return hash((self.path, self._includingSourceFilePath))


# Parser of bindings.yml files
class BindingsFileParser:

    # raise: OSError
    def __init__(self, file):
        self.__file = file
        f = open(file, "r")
        try:
            self.__obj = yaml.safe_load(f)
        finally:
            f.close()

    #raise: yamale.YamaleError
    def validate(self, schemaFile):
        schema = yamale.make_schema(schemaFile)
        data = yamale.make_data(self.__file)
        yamale.validate(schema, data)


    # Set of header files with declarations referenced in the code
    # of synchronization actions, e.g. function calls, constants
    # raise: FatalError
    def getDeclarationSet(self, testDir):
        l = self.__obj["declaration"].split(",")
        i = 0
        while(i < len(l)):
            l[i] = l[i].lstrip().rstrip()
            absPath = os.path.join(testDir, l[i])
            if(not os.path.isfile(absPath)):
                errMsg = "[%s] in declaration list from [%s] is not an existing file"\
                    %(l[i], self.__file)
                raise FatalError(errMsg)
            l[i] = absPath
            i += 1
        return set(l)

    # raise: FatalError
    def getMapping(self, srcDir):
        m = {}
        l = self.__obj["mapping"]
        for e in l:
            f = e.get("file")
            absF = os.path.join(srcDir, f)
            if(not os.path.isfile(absF)):
                errMsg = "file attribute [%s] from [%s] is not an existing file"\
                    %(f, self.__file)
                raise FatalError(errMsg)

            s = e.get("source")
            if(s is None):
                absS = None
            else:
                absS = os.path.join(srcDir, s)
                if(not os.path.isfile(absS)):
                    errMsg = "source attribute [%s] from [%s] is not an existing file"\
                        %(s, self.__file)
                    raise FatalError(errMsg)

            ref = SyncPoint(
                file = absF,
                index = e.get("index"),
                cl = e.get("class"),
                method = e.get("method"),
                source = absS)
            m[ref] = SyncAction(e.get("code"))
        return m


# Class implementing all instrumentation logic (algorithm)
class Instrumentator:

    def __init__(self, argsParser):
        # arguments parser
        self.__argsParser = argsParser

        # set of bindings.yml files
        self.__bindingsFileSet = set()

        # dictionary from synchronization action
        # to set of header files declaring functions,
        # constants and etc. used in that synchronization
        # action
        self.__syncActionToDeclarationSet = {}

        # dictionary from synchronization action
        # to test identifier
        self.__syncActionToTestId = {}

        # dictionary from synchronization point
        # to list of synchronization actions for that point
        self.__syncPointToSyncActionList = {}

        # dictionary from synchronization point to
        # line number in which the synchronization comment
        # for that synchronization point finishes
        self.__syncPointToLine = {}

        # dictionary from file to set of synchronization
        # points from that file
        self.__fileToSyncPointSet = {}

        # dictionary from file to list of compile options
        # used to parse that file
        self.__fileToCompileOptionList = {}

    def __getTestDir(self, bindingsFile):
        return os.path.dirname(bindingsFile)

    def __getTestId(self, testDir):
        return os.path.basename(testDir).upper()

    # parse clade database file and find compilation options
    # for source code modules
    def __lookupCompileOptsInCladeDB(self):
        cladeCmdsFile = self.__argsParser.getCladeFile()
        c = Clade(work_dir=os.path.dirname(cladeCmdsFile), cmds_file=cladeCmdsFile)
        fList = list(self.__fileToSyncPointSet.keys())
        fList.sort(key = lambda f : f.includingSourceFilePath if isinstance(f, HeaderFile) else f.path)
        # go over types of compilation commands
        # (emitted by C and C++ compilers, 2 items in the list)
        for cmdType in ["CXX", "CC"]:
            for cmd in c.get_all_cmds_by_type(cmdType):
                # go over input files of compilation commands
                # i.e. parsed files (usually 1 item in the list)
                for compiledFilePath in cmd["in"]:
                    i = 0
                    # go over sorted file List
                    while(i < len(fList)):
                        f = fList[i]
                        if isinstance(f, HeaderFile):
                            srcFilePath = f.includingSourceFilePath
                        else:
                            srcFilePath = f.path
                        if(compiledFilePath == srcFilePath):
                            compileOptionList = c.get_cmd_opts(cmd["id"])
                            compileOptionList.append('%s=%s' % (CLANG_CUR_WORKING_DIR_OPT, cmd["cwd"]))
                            if isinstance(f,  HeaderFile):
                                compileOptionList.extend(CPP_HEADER_COMPILE_OPTION_LIST)
                            self.__fileToCompileOptionList[f] = compileOptionList
                            fList.pop(i)
                        elif(compiledFilePath < srcFilePath):
                            break
                        else:
                            i += 1
        if(len(fList) > 0):
            errMsg = ["Failed to find compilation command for source code modules:"]
            for f in fList:
                if isinstance(f, HeaderFile):
                    srcFilePath = f.includingSourceFilePath
                else:
                    srcFilePath = f.path
                errMsg.append("\n")
                errMsg.append(srcFilePath)
            raise FatalError(''.join(errMsg))

    def __isKindOfLibclangFunction(self, kind):
        return kind == CursorKind.FUNCTION_DECL \
            or kind == CursorKind.CXX_METHOD \
            or kind == CursorKind.FUNCTION_TEMPLATE \
            or kind == CursorKind.CONSTRUCTOR \
            or kind == CursorKind.DESTRUCTOR \
            or kind == CursorKind.CONVERSION_FUNCTION

    def __isKindOfLibclangClass(self, kind):
        return kind == CursorKind.CLASS_DECL \
            or kind == CursorKind.CLASS_TEMPLATE \
            or kind == CursorKind.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION \
            or kind == CursorKind.STRUCT_DECL

    # Find line numbers of synchronization points
    # in source code modules and header files
    def __lookupSyncPointsWithLibclang(self, file):
        notFound = list(self.__fileToSyncPointSet[file])
        candidates = []

        clangIndex = Index.create()
        tu = clangIndex.parse(file.path, args=self.__fileToCompileOptionList[file])
        for token in tu.get_tokens(extent=tu.cursor.extent):
            if(token.kind != TokenKind.COMMENT):
                continue
            match = re.match(r'\s*/\*\s*@sync\s+([0-9]+).*', token.spelling)
            if(not match):
                continue

            candidates.clear()
            candidates.extend(notFound)

            i = 0
            while(i < len(candidates)):
                if(int(match.group(1)) != candidates[i].index):
                    candidates.pop(i)
                else:
                    i += 1

            i = 0
            while(i < len(candidates)):
                if(candidates[i].hasMethod()):
                    if(not(hasattr(token.cursor, "semantic_parent"))):
                        candidates.pop(i)
                        continue
                    semParent = token.cursor.semantic_parent
                    if(not(self.__isKindOfLibclangFunction(semParent.kind)) \
                        or (semParent.spelling != candidates[i].method)):
                        candidates.pop(i)
                        continue

                    if(candidates[i].hasClass()):
                        if(not(hasattr(semParent, "semantic_parent"))):
                            candidates.pop(i)
                            continue
                        semGrandParent = semParent.semantic_parent
                        if(not(self.__isKindOfLibclangClass(semGrandParent.kind)) \
                            or (semGrandParent.spelling != candidates[i].cl)):
                            candidates.pop(i)
                            continue
                elif(candidates[i].hasClass()):
                    if(not(self.__isKindOfLibclangClass(token.cursor.kind)) \
                        or (token.cursor.spelling != candidates[i].cl)):
                        candidates.pop(i)
                        continue
                i += 1
            for syncPoint in candidates:
                self.__syncPointToLine[syncPoint] = token.extent.end.line + 1
                notFound.remove(syncPoint)

        if(len(notFound) > 0):
            errMsg = ["Failed to find synchronization points:"]
            for syncPoint in notFound:
                errMsg.append("\n")
                errMsg.append(syncPoint.toString())
            raise FatalError(''.join(errMsg))

    def run(self):
        Util.findFilesByName(
            self.__argsParser.getTestSuiteDir(),
            BINDINGS_FILE_NAME,
            self.__bindingsFileSet)
        # go over all bindings.yml files, parse them and fill data structures
        for bindingsFile in self.__bindingsFileSet:
            bindingsParser = BindingsFileParser(bindingsFile)
            bindingsParser.validate(BINDINGS_SCHEMA_FILE)
            testDir = self.__getTestDir(bindingsFile)
            testId = self.__getTestId(testDir)
            declarationSet = bindingsParser.getDeclarationSet(testDir)
            mapping = bindingsParser.getMapping(self.__argsParser.getSrcDir())
            # go over mapping from sync point to sync action in a single bindings.yml
            for r in mapping:
                a = mapping[r]
                self.__syncActionToDeclarationSet[a] = declarationSet
                self.__syncActionToTestId[a] = testId
                if(r not in self.__syncPointToSyncActionList):
                    self.__syncPointToSyncActionList[r] = []
                self.__syncPointToSyncActionList[r].append(a)
                if(r.hasSource()):
                    f = HeaderFile(r.file, r.source)
                else:
                    f = SourceFile(r.file)
                if(f not in self.__fileToSyncPointSet):
                    self.__fileToSyncPointSet[f] = set()
                self.__fileToSyncPointSet[f].add(r)
        self.__lookupCompileOptsInCladeDB()
        frameworkHeaderFileSet = set()
        Util.findFilesByExtension(
            self.__argsParser.getTestSuiteDir(),
            HEADER_FILE_EXTENSION,
            False,
            frameworkHeaderFileSet)

        # Instrumentator includes all headers from runtime/tests/intrusive-tests directory
        # in all places where code is needed to be synced (according to bindings.yml).
        # That is why it includes intrusive_test_option.h file, which contains RuntimeOptions,
        # so framework cannot be used in libpandabase, compiler and etc, but actually
        # intrusive_test_option.h is only needed to initialize testsuite (see Runtime::Create)
        for header in frameworkHeaderFileSet:
            if "intrusive_test_option.h" in header:
              frameworkHeaderFileSet.remove(header)
              break

        # go over map elements (from instrumented file to sync points in that file)
        for file in self.__fileToSyncPointSet:
            # find line numbers of synchronization points
            # in the source code and header files
            self.__lookupSyncPointsWithLibclang(file)
            lines = Util.readFileLines(file.path)
            syncPointList = list(self.__fileToSyncPointSet[file])
            syncPointList.sort(key = lambda spRef : self.__syncPointToLine[spRef])
            syncPointIdx = 0
            # Header files that should be '#included' in the instrumented file
            headerFileSet = set(frameworkHeaderFileSet)
            # go over sync points in a file
            while(syncPointIdx < len(syncPointList)):
                spRef = syncPointList[syncPointIdx]
                syncPointCode = []
                # go over synchronization actions for a synchronization point
                # and insert source code of the synchronization actions into
                # file lines
                for syncAction in self.__syncPointToSyncActionList[spRef]:
                    if(spRef.hasMethod()):
                        if(len(syncPointCode) > 0):
                            syncPointCode.append("\nelse ")
                        syncPointCode.append("if(")
                        syncPointCode.append(CPP_CODE_TO_GET_TEST_ID_IN_PANDA)
                        syncPointCode.append(" == (uint32_t)")
                        syncPointCode.append(self.__syncActionToTestId[syncAction])
                        syncPointCode.append("){\n")
                    syncPointCode.append(syncAction.code)
                    if(spRef.hasMethod()):
                        syncPointCode.append("\n}")
                    syncPointCode.append("\n")
                    headerFileSet.update(self.__syncActionToDeclarationSet[syncAction])
                syncPointCode.insert(0, "#if defined(INTRUSIVE_TESTING)\n")
                syncPointCode.append("#endif\n")
                lines.insert(
                    self.__syncPointToLine[spRef] - 1 + syncPointIdx,
                    ''.join(syncPointCode))
                syncPointIdx += 1
            inclTextList = []
            for header in headerFileSet:
                inclTextList.append("#include \"")
                inclTextList.append(header)
                inclTextList.append("\"\n")
            lines.insert(0, ''.join(inclTextList))
            Util.writeLinesToFile(file.path, lines)


if __name__ == '__main__':
    errMsgSuffix = "Error! Source code instrumentation failed. Reason:"
    try:
        exitCode = 0
        argumentParser = ArgsParser()
        instrumentator = Instrumentator(argumentParser)
        instrumentator.run()
    except yaml.YAMLError as yamlErr:
        exitCode = 1
        print(errMsgSuffix)
        traceback.print_exc(file = sys.stdout, limit=0)
    except yamale.YamaleError as yamaleErr:
        exitCode = 2
        print(errMsgSuffix)
        for res in yamaleErr.results:
            print("Error validating '%s' against schema '%s'\n\t" % (res.data, res.schema))
            for er in res.errors:
                print('\t%s' % er)
    except OSError as osErr:
        exitCode = 3
        errMessage = [osErr.strerror]
        if(hasattr(osErr, 'filename') and (osErr.filename is not None)):
            errMessage.append(": ")
            errMessage.append(osErr.filename)
        if(hasattr(osErr, 'filename2') and (osErr.filename2 is not None)):
            errMessage.append(", ")
            errMessage.append(osErr.filename2)
        print(''.join(errMessage))
    except FatalError as ferr:
        exitCode = 4
        print(errMsgSuffix, ferr.getErrMsg())
    except Exception as err:
        exitCode = 5
        print(errMsgSuffix)
        traceback.print_exc(file = sys.stdout)
    sys.exit(exitCode)
