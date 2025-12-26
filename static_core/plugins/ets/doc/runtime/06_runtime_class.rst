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

.. _Runtime class:

Runtime class
*************

Panda runtime uses ``panda::Class`` to store all necessary language
independent information about class. Virtual table and region for static
fields are embedded to the ``panda::Class`` object so it has variable
size. To get fast access to them ``Class Word`` field of the object
header points to the instance of this class. ``ClassLinker::GetClass``
also return an instance of the ``panda::Class``.

Pointer to the managed class object (instance of ``panda.Class`` or
other in case of plugin-related code) can be obtained using
``panda::Class::GetManagedObject`` method:

.. code:: cpp

   panda::Class *cls = obj->ClassAddr()->GetManagedObject();

We store common runtime information separately from managed object to
give more flexibility for its layout. Disadvantage of this approach is
that we need additional dereference to get ``panda::Class`` from mirror
class and vice versa. But we can use composition to reduce number of
additional dereferences. For example:

.. code:: cpp

   namespace panda::coretypes {
   class Class : public ObjectHeader {

       ... // Mirror fields

       panda::Class klass_;
   };
   }  // namespace panda::coretypes

In this case layout of the ``coretypes::Class`` will be following:

::

   mirror class (`coretypes::Class`) --------> +------------------+ <-+
                                               |    `Mark Word`   |   |
                                               |    `Class Word`  |-----+
                                               +------------------+   | |
                                               |   Mirror fields  |   | |
       panda class (`panda::Class`) ---------> +------------------+ <-|-+
                                               |      ...         |   |
                                               | `Managed Object` |---+
                                               |      ...         |
                                               +------------------+

Note: as ``panda::Class`` object has variable size it must be last in
the mirror class.

Such layout allows to get pointer to the ``panda::Class`` object from
the ``coretypes::Class`` one and vice versa without dereferences if we
know language context and it’s constant (some language specific code):

.. code:: cpp

   auto *managed_class_obj = coretypes::Class::FromRuntimeClass(klass);
   ...
   auto *runtime_class = managed_class_obj->GetRuntimeClass();

Where ``coretypes::Class::FromRuntimeClass`` and
``coretypes::Class::GetRuntimeClass`` are implemented in the following
way:

.. code:: cpp

   namespace panda::coretypes {
   class Class : public ObjectHeader {
       ...

       panda::Class *GetRuntimeClass() {
           return &klass_;
       }

       static constexpr size_t GetRuntimeClassOffset() {
           return MEMBER_OFFSET(Class, klass_);
       }

       static Class *FromRuntimeClass(panda::Class *klass) {
           return reinterpret_cast<Class *>(reinterpret_cast<uintptr_t>(klass) - GetRuntimeClassOffset());
       }

       ...
   };
   }  // namespace panda::coretypes

In common places where language context can be different we can use
``panda::Class::GetManagedObject``. For example:

.. code:: cpp

   auto *managed_class_obj = klass->GetManagedObject();
   ObjectLock lock(managed_class_obj);

Instead of

.. code:: cpp

   ObjectHeader *managed_class_obj;
   switch (klass->GetSourceLang()) {
       case PANDA_ASSEMBLY: {
           managed_class_obj = coretypes::Class::FromRuntimeClass(klass);
           break;
       }
       case PLUGIN_SOURCE_LANG: {
           managed_class_obj = plugin::JClass::FromRuntimeClass(klass);
           break;
       }
       ...
   }
   ObjectLock lock(managed_class_obj);
