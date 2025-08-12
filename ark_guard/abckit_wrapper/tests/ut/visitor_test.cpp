/**
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
#include <gtest/gtest.h>
#include "abckit_wrapper/file_view.h"
#include "test_util/assert.h"
#include "../../library/logger.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/module_static.abc";
abckit_wrapper::FileView fileView;
}  // namespace

class BaseTestVisitor : public abckit_wrapper::ModuleVisitor {
public:
    void AddTargetObjectName(const std::string &objectName)
    {
        this->targetObjectNames_.insert(objectName);
    }

    virtual void InitTargetObjectNames() = 0;

    void Validate()
    {
        this->InitTargetObjectNames();
        ASSERT_EQ(this->objects_.size(), this->targetObjectNames_.size());
        for (const auto &obj : this->objects_) {
            ASSERT_TRUE(this->targetObjectNames_.find(obj->GetFullyQualifiedName()) != this->targetObjectNames_.end());
        }
    }

protected:
    std::vector<abckit_wrapper::Object *> objects_;
    std::set<std::string> targetObjectNames_;
};

class TestVisitor : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override
    {
        ASSERT_TRUE(visitor_ != nullptr);
        visitor_->Validate();
    }

protected:
    std::unique_ptr<BaseTestVisitor> visitor_ = nullptr;
};

class TestLocalModuleVisitor final : public BaseTestVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        LOG_I << "ModuleName:" << module->GetFullyQualifiedName();
        objects_.push_back(module);
        return true;
    }
};

/**
 * @tc.name: module_visitor_001
 * @tc.desc: test visit all local modules of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, module_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalModuleVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalNamespaceVisitor final : public BaseTestVisitor, public abckit_wrapper::NamespaceVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Ns1");
        AddTargetObjectName("module_static.Ns1.Ns2");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->NamespacesAccept(*this);
    }

    bool Visit(abckit_wrapper::Namespace *ns) override
    {
        LOG_I << "NamespaceName:" << ns->GetFullyQualifiedName();
        objects_.push_back(ns);
        return true;
    }
};

/**
 * @tc.name: namespace_visitor_001
 * @tc.desc: test visit all local namespaces of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, namespace_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalNamespaceVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalClassVisitor final : public BaseTestVisitor, public abckit_wrapper::ClassVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Ns1.Interface2");
        AddTargetObjectName("module_static.Ns1.Color2");
        AddTargetObjectName("module_static.Ns1.Color2[]");
        AddTargetObjectName("module_static.Ns1.ClassB");
        AddTargetObjectName("module_static.ClassA");
        AddTargetObjectName("module_static.Interface1");
        AddTargetObjectName("module_static.Color1");
        AddTargetObjectName("module_static.Color1[]");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->ClassesAccept(*this);
    }

    bool Visit(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }
};

/**
 * @tc.name: class_visitor_001
 * @tc.desc: test visit all local classes of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, class_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalClassVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalLeafClassVisitor final : public BaseTestVisitor, public abckit_wrapper::ClassVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Ns1.Color2");
        AddTargetObjectName("module_static.Ns1.Color2[]");
        AddTargetObjectName("module_static.Ns1.ClassB");
        AddTargetObjectName("module_static.ClassA");
        AddTargetObjectName("module_static.Color1");
        AddTargetObjectName("module_static.Color1[]");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return true;
    }

    bool Visit(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }
};

/**
 * @tc.name: class_visitor_002
 * @tc.desc: test visit all local leaf classes of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, class_visitor_002, TestSize.Level1)
{
    auto testVisitor = std::make_unique<TestLocalLeafClassVisitor>();
    abckit_wrapper::ClassVisitor *classVisitor = testVisitor.get();
    ASSERT_TRUE(fileView.ModulesAccept(classVisitor->Wrap<abckit_wrapper::LeafClassVisitor>()
                                           .Wrap<abckit_wrapper::ModuleClassVisitor>()
                                           .Wrap<abckit_wrapper::LocalModuleFilter>()));
    visitor_ = std::move(testVisitor);
}

class TestLocalModuleChildVisitor final : public BaseTestVisitor, public abckit_wrapper::ChildVisitor {
public:
    void InitTargetObjectNames() override
    {
        // Namespaces
        AddTargetObjectName("module_static.Ns1");
        // Classes
        AddTargetObjectName("module_static.ClassA");
        AddTargetObjectName("module_static.Color1");
        AddTargetObjectName("module_static.Color1[]");
        AddTargetObjectName("module_static.Interface1");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->ChildrenAccept(*this);
    }

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override
    {
        LOG_I << "NamespaceName:" << ns->GetFullyQualifiedName();
        objects_.push_back(ns);
        return true;
    }

    bool VisitClass(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }
};

/**
 * @tc.name: child_visitor_001
 * @tc.desc: test visit all directly child of the local module
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, child_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalModuleChildVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalNamespaceChildVisitor final : public BaseTestVisitor, public abckit_wrapper::ChildVisitor {
public:
    void InitTargetObjectNames() override
    {
        // Namespaces
        AddTargetObjectName("module_static.Ns1.Ns2");
        // Classes
        AddTargetObjectName("module_static.Ns1.ClassB");
        AddTargetObjectName("module_static.Ns1.Color2");
        AddTargetObjectName("module_static.Ns1.Color2[]");
        AddTargetObjectName("module_static.Ns1.Interface2");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return true;
    }

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override
    {
        LOG_I << "NamespaceName:" << ns->GetFullyQualifiedName();
        objects_.push_back(ns);
        return true;
    }

    bool VisitClass(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }
};

/**
 * @tc.name: child_visitor_002
 * @tc.desc: test visit all directly child of the local namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, child_visitor_002, TestSize.Level1)
{
    auto testVisitor = std::make_unique<TestLocalNamespaceChildVisitor>();
    const auto ns = fileView.Get<abckit_wrapper::Namespace>("module_static.Ns1");
    ASSERT_TRUE(ns.has_value());
    ASSERT_TRUE(ns.value()->ChildrenAccept(*testVisitor));
    visitor_ = std::move(testVisitor);
}

class TestClassAFilter final : public abckit_wrapper::ClassFilter {
public:
    explicit TestClassAFilter(ClassVisitor &visitor) : ClassFilter(visitor) {}

    bool Accepted(abckit_wrapper::Class *clazz) override
    {
        return clazz->GetFullyQualifiedName() == "module_static.ClassA";
    }
};
