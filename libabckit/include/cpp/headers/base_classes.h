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

#include "./config.h"

#include <memory>

namespace abckit {

// Interface to provide global API-related features,
// a base for every API class defined
/**
 * @brief Entity
 */
class Entity {
public:
    /**
     * @brief Constructor
     * @param other
     */
    Entity(const Entity &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Entity
     */
    Entity &operator=(const Entity &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    Entity(Entity &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Entity
     */
    Entity &operator=(Entity &&other) = default;

    Entity() = default;
    virtual ~Entity() = default;

    /**
     * @brief Get api config
     * @return ApiConfig
     */
    virtual const ApiConfig *GetApiConfig() const = 0;
};

// View - value semantics
/**
 * @brief View
 */
template <typename T>
class View : public Entity {
public:
    /**
     * Operator ==
     * @param rhs
     * @return bool
     */
    bool operator==(const View<T> &rhs)
    {
        return GetView() == rhs.GetView();
    }

protected:
    /**
     * Constructor
     * @param ...a
     */
    template <typename... Args>
    explicit View(Args &&...a) : view_(std::forward<Args>(a)...)
    {
    }
    // Can move and copy views

    /**
     * @brief Constructor
     * @param other
     */
    View(const View &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return View
     */
    View &operator=(const View &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    View(View &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return View
     */
    View &operator=(View &&other) = default;

protected:
    ~View() override = default;

    /**
     * Get view
     * @return T
     */
    T GetView() const
    {
        return view_;
    }

    /**
     * Set view
     * @param newView
     */
    void SetView(T newView)
    {
        view_ = newView;
    }

private:
    T view_;
};

// Resource - ptr semantics
/**
 * @brief Resource
 */
template <typename T>
class Resource : public Entity {
public:
    // No copy for resources
    /**
     * @brief Deleted constructor
     * @param other
     */
    Resource(Resource &other) = delete;

    /**
     * @brief Deleted constructor
     * @return Resource&
     * @param other
     */
    Resource &operator=(Resource &other) = delete;

protected:
    /**
     * @brief Constructor
     * @param d
     * @param ...a
     */
    template <typename... Args>
    explicit Resource(std::unique_ptr<IResourceDeleter> d, Args &&...a)
        : deleter_(std::move(d)), resource_(std::forward<Args>(a)...)
    {
    }

    /**
     * @brief Constructor
     * @param ...a
     */
    template <typename... Args>
    explicit Resource(Args &&...a) : resource_(std::forward<Args>(a)...)
    {
    }

    // Resources are movable
    /**
     * @brief Constructor
     * @param other
     */
    Resource(Resource &&other)
    {
        released_ = false;
        resource_ = other.ReleaseResource();
    };

    /**
     * @brief
     * @param other
     * @return Resource&
     */
    Resource &operator=(Resource &&other)
    {
        released_ = false;
        resource_ = other.ReleaseResource();
        return *this;
    };

    /**
     * @brief Release resource
     * @return `T`
     */
    T ReleaseResource()
    {
        released_ = true;
        return resource_;
    }

    /**
     * @brief Get resource
     * @return `T`
     */
    T GetResource() const
    {
        return resource_;
    }

    /**
     * @brief Destructor
     */
    ~Resource() override
    {
        if (!released_) {
            deleter_->DeleteResource();
        }
    };

    /**
     * @brief Set deleter
     * @param deleter
     */
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
