/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_BASE_CLASSES_H
#define CPP_ABCKIT_BASE_CLASSES_H

#include "cpp/headers/config.h"
#include "cpp/headers/declarations.h"

#include <memory>

namespace abckit {

// Interface to provide global API-related features,
// a base for every API class defined
class Entity {
public:
    Entity(const Entity &other) = default;
    Entity &operator=(const Entity &other) = default;
    Entity(Entity &&other) = default;
    Entity &operator=(Entity &&other) = default;

protected:
    Entity() = default;
    virtual ~Entity() = default;

    virtual const ApiConfig *GetApiConfig() const = 0;
};

// View - value semantics
template <typename T>
class View : public Entity {
public:
    template <typename... Args>
    explicit View(Args &&...a) : view_(std::forward<Args>(a)...)
    {
    }
    // Can move and copy views
    View(const View &other) = default;
    View &operator=(const View &other) = default;
    View(View &&other) = default;
    View &operator=(View &&other) = default;

    bool operator==(const View<T> &rhs)
    {
        return GetView() == rhs.GetView();
    }

protected:
    ~View() override = default;

    T GetView() const
    {
        return view_;
    }

    void SetView(T newView)
    {
        view_ = newView;
    }

private:
    T view_;
};

// Resource - ptr semantics
template <typename T>
class Resource : public Entity {
public:
    template <typename... Args>
    explicit Resource(std::unique_ptr<IResourceDeleter> d, Args &&...a)
        : deleter_(std::move(d)), resource_(std::forward<Args>(a)...)
    {
    }

    template <typename... Args>
    explicit Resource(Args &&...a) : resource_(std::forward<Args>(a)...)
    {
    }

    // No copy for resources
    Resource(Resource &other) = delete;
    Resource &operator=(Resource &other) = delete;
    // Resources are movable
    Resource(Resource &&other)
    {
        released_ = false;
        resource_ = other.ReleaseResource();
    };
    Resource &operator=(Resource &&other)
    {
        released_ = false;
        resource_ = other.ReleaseResource();
        return *this;
    };

    T ReleaseResource()
    {
        released_ = true;
        return resource_;
    }

    T GetResource() const
    {
        return resource_;
    }

protected:
    ~Resource() override
    {
        if (!released_) {
            deleter_->DeleteResource();
        }
    };
    void SetDeleter(std::unique_ptr<IResourceDeleter> deleter)
    {
        deleter_ = std::move(deleter);
    }

private:
    T resource_;
    std::unique_ptr<IResourceDeleter> deleter_ = std::make_unique<DefaultResourceDeleter>();
    bool released_ = false;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_BASE_CLASSES_H
