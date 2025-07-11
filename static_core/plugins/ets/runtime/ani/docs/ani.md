## 1. Version Information
### GetVersion
`ani_status (*GetVersion)(ani_env *env, uint32_t *result);`

This function retrieves the version information and stores it in the result parameter.

## 2. Class Operations
### FindClass 
`ani_status (*FindClass)(ani_env *env, const char *class_descriptor, ani_class *result);`

This function locates a class based on its descriptor and stores it in the result parameter.

**PARAMETERS:**
env: A pointer to the environment structure.
class_descriptor: The descriptor of the class to find.
result: A pointer to the class to be populated.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Found class object success.

ANI_NOT_FOUND: env HasPendingException && Exception == CLASS_NOT_FOUND_EXCEPTION

ANI_PENDING_ERROR: env HasPendingException && Exception != CLASS_NOT_FOUND_EXCEPTION

### Class_FindStaticMethod
`ani_status (*Class_FindStaticMethod)(ani_env *env, ani_type type, const char *name, const char *signature, ani_static_method *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
name: The name of the static method to retrieve.
signature: The signature of the static method to retrieve.
result: A pointer to store the retrieved static method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

- `ANI_OK`: Find a static method in the specified class and convert it to an object of type `ani_datic_sthod` to return success.

### Class_FindMethod
`ani_status (*Class_FindMethod)(ani_env *env, ani_type type, const char *name, const char *signature, ani_method *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
name: The name of the method to retrieve.
signature: The signature of the method to retrieve.
result: A pointer to store the retrieved method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

- `ANI_OK`: Find a method in the specified class and convert it to an object of type `ani_datic_sthod` to return success.

### Class_BindNativeMethods
`ani_status (*Class_BindNativeMethods)(ani_env *env, ani_class cls, const ani_native_function *methods, ani_size nr_methods);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to which the native methods will be bound.
methods: A pointer to an array of native methods to bind.
nr_methods: The number of native methods in the array.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

- `ANI_OK`: Virtual machine successfully associates local methods with classes.
- `ANI_INVAILD_ARGS`: `cls == nullptr || method == nullptr || nr_methods == nullptr`.

### type_GetSuperClass
`ani_status (*Type_GetSuperClass)(ani_env *env, ani_type type, ani_class *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
type: The type for which to retrieve the superclass.
result: A pointer to the superclass to be populated.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### type_IsAssignableFrom
`ani_status (*Type_IsAssignableFrom)(ani_env *env, ani_type from_type, ani_type to_type, ani_boolean *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
from_type: The source type.
to_type: The target type.
result A pointer to a boolean indicating assignability.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### FindModule
`ani_status (*FindModule)(ani_env *env, const char *module_descriptor, ani_module *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
module_descriptor: The descriptor of the module to find.
result: A pointer to the module to be populated.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### FindNamespace
`ani_status (*FindNamespace)(ani_env *env, const char *namespace_descriptor, ani_namespace *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
namespace_descriptor: The descriptor of the namespace to find.
result: A pointer to the namespace to be populated.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

**PARAMETERS:**
env: A pointer to the environment structure.
class_descriptor: The descriptor of the class to find.
result: A pointer to the class to be populated.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### FindEnum
`ani_status (*FindEnum)(ani_env *env, const char *enum_descriptor, ani_enum *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
namespace_descriptor: The descriptor of the namespace to find.
result: A pointer to the namespace to be populated.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_FindStaticMethod
`ani_status (*Class_FindStaticMethod)(ani_env *env, ani_class cls, const char *name, const char *signature, ani_static_method *result);`

This function retrieves the specified static method by name and signature from the given class.

**PARAMETERS:**

env: a pointer to the environment structure.

cls: The class to query.

name: The name of the static method to retrieve.

signature: The signature of the static method to retrieve.

result: A pointer to store the retrieved static method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

ANI_OK: Find a static method in the specified class and convert it to an object of type ani_datic_sthod to return success.

### Class_FindMethod
`ani_status (*Class_FindMethod)(ani_env *env, ani_class cls, const char *name, const char *signature, ani_method *result);`

This function retrieves the specified method by name and signature from the given class.

**PARAMETERS:**

env: A pointer to the environment structure.

cls: The class to query.

name: The name of the method to retrieve.

signature: The signature of the method to retrieve.

result: A pointer to store the retrieved method.

**RETURNS:**

Returns a status code of type ani_status indicating success or failure.

Return ANI_OK : get a reference to the return value of a method success

Return ANI_INVALID_ARGS : cls == nullptr || name == nullptr || result == nullptr

Return ANI_PENDING_ERROR : has a pending exception

Return ANI_ERROR : class not found

Return ANI_NOT_FOUND : method not found | method is static

### Class_BindNativeMethods
`ani_status (*Class_BindNativeMethods)(ani_env *env, ani_class cls, const ani_native_function *methods, ani_size nr_methods);`

This function binds an array of native methods to the specified class.

**PARAMETERS:**

env: A pointer to the environment structure.

cls: The object on which the method is to be called.

methods: A pointer to an array of native methods to bind.

nr_methods: The number of native methods in the array.

**RETURNS:**

Returns a status code of type ani_status indicating success or failure.

Return ANI_OK : binds an array of native methods to the specified class success

Return ANI_INVALID_ARGS : cls == nullptr || methods == nullptr

Return ANI_NOT_FOUND : method not found | method is not native

## 3. Exceptions
### ExistUnhandledError
`ani_status (*ExistUnhandledError)(ani_env *env, ani_boolean *result);`

**PARAMETERS:**
env:A pointer to the tuple to be populated.
result: A pointer to a boolean indicating if unhandled errors exist.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### DescribeError
`ani_status (*DescribeError)(ani_env *env);`

**PARAMETERS:**
env: A pointer to the environment structure.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### ThrowError
`ani_status (*ThrowError)(ani_env *env, ani_error err);`

**PARAMETERS:**
env: A pointer to the environment structure.
err: The error to throw.

**RETURNS:**
 a status code of type `ani_status` indicating success or failure.

### GetUnhandledError
`ani_status (*GetUnhandledError)(ani_env *env, ani_error *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
result: A pointer to store the unhandled error.

**RETURNS:**
a status code of type `ani_status` indicating success or failure.
	
### Abort
`ani_status (*Abort)(ani_env *env, const char *message);`

**PARAMETERS:**
env: A pointer to the environment structure.
message: The error message to display on termination.

**RETURNS:**
Does not return; the process terminates.

### ResetError
`ani_status (*ResetError)(ani_env *env);`

This function clears the error state in the current environment.

**PARAMETERS:**

env: A pointer to the environment structure.

**RETURNS:**

Returns a status code of type ani_status indicating success or failure.

ANI_OK: reset error success

### ExistUnhandledError
`ani_status (*ExistUnhandledError)(ani_env *env, ani_boolean *result);`

This function determines if there are unhandled errors in the current environment.

**PARAMETERS:**

env: A pointer to the environment structure.

result: A pointer to a boolean indicating if unhandled errors exist.

**RETURNS:**

Returns a status code of type ani_status indicating success or failure.

ANI_INVALID_ARGS: result == nullptr

ANI_OK : UxistUnhandledError excute success.

## 4. Global and Local References
### GlobalReference_Create
`ani_status (*GlobalReference_Create)(ani_env *env, ani_ref ref, ani_gref *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
ref: The local reference to convert to a global reference.
result: A pointer to store the created global reference.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

- `ANI_OK`: Creates a global reference from a local reference success.
- `ANI_INVAILD_ARGS`: `ref == nullptr || result == nullptr`.

### Reference_StrictEquals
`ani_status (*Reference_StrictEquals)(ani_env *env, ani_ref ref0, ani_ref ref1, ani_boolean *result);`

### GlobalReference_Delete
`ani_status (*GlobalReference_Delete)(ani_env *env, ani_gref ref);`

### Reference_Delete
`ani_status (*Reference_Delete)(ani_env *env, ani_ref ref);`

### EnsureEnoughReferences
`ani_status (*EnsureEnoughReferences)(ani_env *env, ani_size nr_refs);`

GlobalReference_Create
This function creates a global reference from a local reference with an initial reference count.

**PARAMETERS:**

env: A pointer to the environment structure.

ref: The local reference to convert to a global reference.

initial_refcount: The initial reference count for the global reference.

result: A pointer to store the created global reference.

**RETURNS:**

Returns a status code of type ani_status indicating success or failure.

ANI_INVALID_ARGS: ref == nullptr || result == nullptr

ANI_OK : global reference create successful

## 5. Weak Global References
### WeakReference_Create
`ani_status (*WeakReference_Create)(ani_env *env, ani_ref ref, ani_wref *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
ref: The local reference to convert to a weak reference.
result: A pointer to store the created weak reference.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
(This is a temporary implementation, it needs to be redone.)

### WeakReference_Delete
`ani_status (*WeakReference_Delete)(ani_env *env, ani_wref wref);`

### WeakReference_GetReference
`ani_status (*WeakReference_GetReference)(ani_env *env, ani_wref wref, ani_ref *result);`

## 6. Object Operations
### Object_InstanceOf
`ani_status (*Object_InstanceOf)(ani_env *env, ani_object object, ani_type type, ani_boolean *result);`


**PARAMETERS:**
env: A pointer to the environment structure.
object: The object to check.
type: The type to compare against.
result: A pointer to store the boolean result (true if the object is an instance of the type, false otherwise).

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetType
`ani_status (*Object_GetType)(ani_env *env, ani_object object, ani_type *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object whose type is to be retrieved.
result: A pointer to store the retrieved type.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_New
`ani_status (*Object_New)(ani_env *env, ani_class cls, ani_method method, ...);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class of the object to create.
method: The constructor method to invoke.
...: Variadic arguments to pass to the constructor method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Reference_IsNull
`ani_status (*Reference_IsNull)(ani_env *env, ani_ref ref, ani_boolean *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
ref: The reference to check.
result: A pointer to a boolean indicating if the reference is null.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Reference_IsUndefined
`ani_status (*Reference_IsUndefined)(ani_env *env, ani_ref ref, ani_boolean *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
ref: The reference to check.
result: A pointer to a boolean indicating if the reference is undefined.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Reference_IsNullishValue
`ani_status (*Reference_IsNullishValue)(ani_env *env, ani_ref ref, ani_boolean *result);`

### Reference_Equals
`ani_status (*Reference_Equals)(ani_env *env, ani_ref ref0, ani_ref ref1, ani_boolean *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
ref0: The first reference to compare.
ref1: The second reference to compare.
result: A pointer to a boolean indicating if the references are equal.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Array_New_Ref
`ani_status (*Array_New_Ref)(ani_env *env, ani_size length, ani_ref *initial_array, ani_fixedarray_ref *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
initial_array: An optional array of references to initialize the fixed array. Can be null.
result: A pointer to store the created fixed array of references.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_New_A
`ani_status (*Object_New_A)(ani_env *env, ani_class cls, ani_method method, const ani_value *args);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class of the object to create.
method: The constructor method to invoke.
args: An array of arguments to pass to the constructor method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_New_V
`ani_status (*Object_New_V)(ani_env *env, ani_class cls, ani_method method, va_list args);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class of the object to create.
method: The constructor method to invoke.
args: A `va_list` of arguments to pass to the constructor method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_InstanceOf
`ani_status (*Object_InstanceOf)(ani_env *env, ani_object object, ani_type type, ani_boolean *result);`

This function checks whether the given object is an instance of the specified type.

**PARAMETERS:**

env: A pointer to the environment structure.

object: The object to check.

type: The type to compare against.

result: A pointer to store the boolean result (true if the object is an instance of the type, false otherwise).

**RETURNS:**

Returns a status code of type ani_status indicating success or failure.

ANI_INVALID_ARGS: object== nullptr || type == nullptr

ANI_OK : a is instance of b

## 7. Accessing Fileds of Objects
### Object_GetFieldByName_Ref
`ani_status (*Object_GetFieldByName_Ref)(ani_env *env, ani_object object, const char *name, ani_ref *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the reference value from.
result: A pointer to store the retrieved reference value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

- `ANI_OK`: Retrieves the reference value of the specified field from the given object by its name success.
- `ANI_INVAILD_ARGS`: `object == nullptr || name == nullptr || result == nullptr`.
- `ANI_INVALID_TYPE`: The specified name type of the field does not match the expected `ani_def` type.
- `ANI_NOT_FOUND`: The specified name field does not exist.

### Object_SetProperty_Boolean
`ani_status (*Object_SetProperty_Boolean)(ani_env *env, ani_object object, ani_property property, ani_boolean value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the boolean value to.
value: The boolean value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetProperty_Char
`ani_status (*Object_SetProperty_Char)(ani_env *env, ani_object object, ani_property property, ani_char value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the char value to.
value: The char value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetProperty_Byte
`ani_status (*Object_SetProperty_Byte)(ani_env *env, ani_object object, ani_property property, ani_byte value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the byte value to.
value: The byte value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetProperty_Int
`ani_status (*Object_SetProperty_Int)(ani_env *env, ani_object object, ani_property property, ani_int value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the integer value to.
value: The integer value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetProperty_Long
`ani_status (*Object_SetProperty_Long)(ani_env *env, ani_object object, ani_property property, ani_long value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the long value to.
value: The long value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetProperty_Float
`ani_status (*Object_SetProperty_Float)(ani_env *env, ani_object object, ani_property property, ani_float value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the float value to.
value: The float value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetProperty_Double
`ani_status (*Object_SetProperty_Double)(ani_env *env, ani_object object, ani_property property, ani_double value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the double value to.
value: The double value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetProperty_Ref
`ani_status (*Object_SetProperty_Ref)(ani_env *env, ani_object object, ani_property property, ani_ref value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
property: The property to set the reference value to.
value: The reference value to assign to the property.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Boolean
`ani_status (*Object_GetPropertyByName_Boolean)(ani_env *env, ani_object object, const char *name, ani_boolean *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the boolean value from.
result: A pointer to store the retrieved boolean value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Char
`ani_status (*Object_GetPropertyByName_Char)(ani_env *env, ani_object object, const char *name, ani_char *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the char value from.
result: A pointer to store the retrieved char value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Byte
`ani_status (*Object_GetPropertyByName_Byte)(ani_env *env, ani_object object, const char *name, ani_byte *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the byte value from.
result: A pointer to store the retrieved byte value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### 
`ani_status (*Object_GetPropertyByName_Short)(ani_env *env, ani_object object, const char *name, ani_short *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the short value from.
result: A pointer to store the retrieved short value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Int
`ani_status (*Object_GetPropertyByName_Int)(ani_env *env, ani_object object, const char *name, ani_int *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the integer value from.
result: A pointer to store the retrieved integer value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Long
`ani_status (*Object_GetPropertyByName_Long)(ani_env *env, ani_object object, const char *name, ani_long *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the long value from.
result: A pointer to store the retrieved long value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Float
`ani_status (*Object_GetPropertyByName_Float)(ani_env *env, ani_object object, const char *name, ani_float *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the float value from.
result: A pointer to store the retrieved float value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Double
`ani_status (*Object_GetPropertyByName_Double)(ani_env *env, ani_object object, const char *name, ani_double *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the double value from.
result: A pointer to store the retrieved double value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetPropertyByName_Ref
`ani_status (*Object_GetPropertyByName_Ref)(ani_env *env, ani_object object, const char *name, ani_ref *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the property.
name: The name of the property to retrieve the reference value from.
result: A pointer to store the retrieved reference value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_GetRequired
`ani_status (*Class_GetRequired)(ani_env *env, ani_class cls, ani_class *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
result: A pointer to store the required class representation.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_FindField
`ani_status (*Class_FindField)(ani_env *env, ani_class cls, const char *name, ani_field *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
name: The name of the field to retrieve.
result: A pointer to store the retrieved field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_FindSetter
`ani_status (*Class_FindSetter)(ani_env *env, ani_class cls, const char *name, ani_method *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
name: The name of the property whose setter is to be retrieved.
result: A pointer to store the retrieved setter method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_FindGetter
`ani_status (*Class_FindGetter)(ani_env *env, ani_class cls, const char *name, ani_method *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
name: The name of the property whose getter is to be retrieved.
result: A pointer to store the retrieved getter method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_FindIndexableGetter
`ani_status (*Class_FindIndexableGetter)(ani_env *env, ani_class cls, const char *signature, ani_method *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
signature: The signature of the indexable getter to retrieve.
result: A pointer to store the retrieved indexable getter method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_FindIndexableSetter
`ani_status (*Class_FindIndexableSetter)(ani_env *env, ani_class cls, const char *signature, ani_method *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
signature: The signature of the indexable setter to retrieve.
result: A pointer to store the retrieved indexable setter method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_FindIterator
`ani_status (*Class_FindIterator)(ani_env *env, ani_class cls, ani_method *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class to query.
result: A pointer to store the retrieved iterator method.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Boolean
`ani_status (*Object_GetField_Boolean)(ani_env *env, ani_object object, ani_field field, ani_boolean *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the boolean value from.
result: A pointer to store the retrieved boolean value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Char
`ani_status (*Object_GetField_Char)(ani_env *env, ani_object object, ani_field field, ani_char *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the char value from.
result: A pointer to store the retrieved char value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Byte
`ani_status (*Object_GetField_Byte)(ani_env *env, ani_object object, ani_field field, ani_byte *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the byte value from.
result: A pointer to store the retrieved byte value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Short
`ani_status (*Object_GetField_Short)(ani_env *env, ani_object object, ani_field field, ani_short *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the short value from.
result: A pointer to store the retrieved short value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Int
`ani_status (*Object_GetField_Int)(ani_env *env, ani_object object, ani_field field, ani_int *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the integer value from.
result: A pointer to store the retrieved integer value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Long
`ani_status (*Object_GetField_Long)(ani_env *env, ani_object object, ani_field field, ani_long *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the long value from.
result: A pointer to store the retrieved long value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Float
`ani_status (*Object_GetField_Float)(ani_env *env, ani_object object, ani_field field, ani_float *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the float value from.
result: A pointer to store the retrieved float value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Double
`ani_status (*Object_GetField_Double)(ani_env *env, ani_object object, ani_field field, ani_double *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the double value from.
result: A pointer to store the retrieved double value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetField_Ref
`ani_status (*Object_GetField_Ref)(ani_env *env, ani_object object, ani_field field, ani_ref *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to retrieve the reference value from.
result: A pointer to store the retrieved reference value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Boolean
`ani_status (*Object_SetField_Boolean)(ani_env *env, ani_object object, ani_field field, ani_boolean value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the boolean value to.
value: The boolean value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Char
`ani_status (*Object_SetField_Char)(ani_env *env, ani_object object, ani_field field, ani_char value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the char value to.
value: The char value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Byte
`ani_status (*Object_SetField_Byte)(ani_env *env, ani_object object, ani_field field, ani_byte value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the byte value to.
value: The byte value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Short
`ani_status (*Object_SetField_Short)(ani_env *env, ani_object object, ani_field field, ani_short value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the short value to.
value: The short value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Int
`ani_status (*Object_SetField_Int)(ani_env *env, ani_object object, ani_field field, ani_int value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the integer value to.
value: The integer value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Long
`ani_status (*Object_SetField_Long)(ani_env *env, ani_object object, ani_field field, ani_long value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the long value to.
value: The long value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Float
`ani_status (*Object_SetField_Float)(ani_env *env, ani_object object, ani_field field, ani_float value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the float value to.
value: The float value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Double
`ani_status (*Object_SetField_Double)(ani_env *env, ani_object object, ani_field field, ani_double value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the double value to.
value: The double value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetField_Ref
`ani_status (*Object_SetField_Ref)(ani_env *env, ani_object object, ani_field field, ani_ref value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
field: The field to set the reference value to.
value: The reference value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Boolean
`ani_status (*Object_GetFieldByName_Boolean)(ani_env *env, ani_object object, const char *name, ani_boolean *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the boolean value from.
result: A pointer to store the retrieved boolean value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Char
`ani_status (*Object_GetFieldByName_Char)(ani_env *env, ani_object object, const char *name, ani_char *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the char value from.
result: A pointer to store the retrieved char value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Byte
`ani_status (*Object_GetFieldByName_Byte)(ani_env *env, ani_object object, const char *name, ani_byte *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the byte value from.
result: A pointer to store the retrieved byte value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Short
`ani_status (*Object_GetFieldByName_Short)(ani_env *env, ani_object object, const char *name, ani_short *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the short value from.
result: A pointer to store the retrieved short value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Int
`ani_status (*Object_GetFieldByName_Int)(ani_env *env, ani_object object, const char *name, ani_int *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the integer value from.
result: A pointer to store the retrieved integer value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Long
`ani_status (*Object_GetFieldByName_Long)(ani_env *env, ani_object object, const char *name, ani_long *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the long value from.
result: A pointer to store the retrieved long value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Float
`ani_status (*Object_GetFieldByName_Float)(ani_env *env, ani_object object, const char *name, ani_float *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the float value from.
result: A pointer to store the retrieved float value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_GetFieldByName_Double
`ani_status (*Object_GetFieldByName_Double)(ani_env *env, ani_object object, const char *name, ani_double *result);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to retrieve the double value from.
result: A pointer to store the retrieved double value.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Boolean
`ani_status (*Object_SetFieldByName_Boolean)(ani_env *env, ani_object object, const char *name, ani_boolean value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the boolean value to.
value: The boolean value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Char
`ani_status (*Object_SetFieldByName_Char)(ani_env *env, ani_object object, const char *name, ani_char value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the char value to.
value: The char value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Byte
`ani_status (*Object_SetFieldByName_Byte)(ani_env *env, ani_object object, const char *name, ani_byte value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the byte value to.
value: The byte value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Short
`ani_status (*Object_SetFieldByName_Short)(ani_env *env, ani_object object, const char *name, ani_short value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the short value to.
value: The short value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Int
`ani_status (*Object_SetFieldByName_Int)(ani_env *env, ani_object object, const char *name, ani_int value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the integer value to.
value: The integer value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Long
`ani_status (*Object_SetFieldByName_Long)(ani_env *env, ani_object object, const char *name, ani_long value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the long value to.
value: The long value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Float
`ani_status (*Object_SetFieldByName_Float)(ani_env *env, ani_object object, const char *name, ani_float value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the float value to.
value: The float value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Double
`ani_status (*Object_SetFieldByName_Double)(ani_env *env, ani_object object, const char *name, ani_double value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the double value to.
value: The double value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Object_SetFieldByName_Ref
`ani_status (*Object_SetFieldByName_Ref)(ani_env *env, ani_object object, const char *name, ani_ref value);`

**PARAMETERS:**
env: A pointer to the environment structure.
object: The object containing the field.
name: The name of the field to set the reference value to.
value: The reference value to assign to the field.

**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

## 8. Calling Instance Methods

### Object_CallMethod_Double  

`ani_status (*Object_CallMethod_Double)(ani_env *env, ani_object object, ani_method method, ani_double *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the double return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Calls the specified method of an object using variadic arguments and retrieves a double result success.
ANI_INVAILD_ARGS: object == nullptr || method == nullptr || result == nullptr.

### Object_CallMethod_Double_A  

`ani_status (*Object_CallMethod_Double_A)(ani_env *env, ani_object object, ani_method method, ani_double *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the double return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Calls the specified method of an object using variadic arguments and retrieves a integer result success.
ANI_INVAILD_ARGS`: `object == nullptr || method == nullptr || result == nullptr.

### Object_CallMethod_Double_V  

`ani_status (*Object_CallMethod_Double_V)(ani_env *env, ani_object object, ani_method method, ani_double *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the reference return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Calls the specified method of an object using variadic arguments and retrieves a integer result success.
ANI_INVAILD_ARGS`: object == nullptr || method == nullptr || result == nullptr.

### Object_CallMethod_Int  

`ani_status (*Object_CallMethod_Int)(ani_env *env, ani_object object, ani_method method, ani_int *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the integer return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Calls the specified method of an object using variadic arguments and retrieves a integer result success.
ANI_INVAILD_ARGS`: `object == nullptr || method == nullptr || result == nullptr.

### Object_CallMethod_Int_A  

`ani_status (*Object_CallMethod_Int_A)(ani_env *env, ani_object object, ani_method method, ani_int *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the integer return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Calls the specified method of an object using variadic arguments and retrieves a integer result success.
ANI_INVAILD_ARGS`: `object == nullptr || method == nullptr || result == nullptr.

### Object_CallMethod_Int_V  

`ani_status (*Object_CallMethod_Int_V)(ani_env *env, ani_object object, ani_method method, ani_int *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the long return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Calls the specified method of an object using variadic arguments and retrieves a integer result success.
ANI_INVAILD_ARGS: object == nullptr || method == nullptr || result == nullptr.

### Object_CallMethod_Boolean  

`ani_status (*Object_CallMethod_Boolean)(ani_env *env, ani_object object, ani_method method, ani_boolean *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the boolean return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Boolean_V  

`ani_status (*Object_CallMethod_Boolean_V)(ani_env *env, ani_object object, ani_method method, ani_boolean *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the boolean return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Boolean_A  

`ani_status (*Object_CallMethod_Boolean_A)(ani_env *env, ani_object object, ani_method method, ani_boolean *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the boolean return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Void  

`ani_status (*Object_CallMethod_Void)(ani_env *env, ani_object object, ani_method method, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Void_A  

`ani_status (*Object_CallMethod_Void_A)(ani_env *env, ani_object object, ani_method method, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Void_V  

`ani_status (*Object_CallMethod_Void_V)(ani_env *env, ani_object object, ani_method method, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Long  

`ani_status (*Object_CallMethod_Long)(ani_env *env, ani_object object, ani_method method, ani_long *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the long return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Long_A  

`ani_status (*Object_CallMethod_Long_A)(ani_env *env, ani_object object, ani_method method, ani_long *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the long return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Long_V  

`ani_status (*Object_CallMethod_Long_V)(ani_env *env, ani_object object, ani_method method, ani_long *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the long return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### FunctionalObject_Call  

`ani_status (*FunctionalObject_Call)(ani_env *env, ani_fn_object fn, ani_size argc, ani_ref *argv, ani_ref *result);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The functional object to invoke.
argc: The number of arguments being passed to the functional object.
argv: A pointer to an array of references representing the arguments. Can be null if `argc` is 0.
result: A pointer to store the result of the invocation. Can be null if the functional object does not return a value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Module_BindNativeFunctions  

`ani_status (*Module_BindNativeFunctions)(ani_env *env, ani_module module, const ani_native_function *functions, ani_size nr_functions);`  
**PARAMETERS:**
env: A pointer to the environment structure.
module: The module to which the native functions will be bound.
functions: A pointer to an array of native functions to bind.
nr_functions: The number of native functions in the array.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
ANI_OK: Binds an array of native functions to the specified module success && `nrFunctions == 0.
ANI_INVAILD_ARGS: module == nullptr || functions == nullptr || nr_functions == nullptr.

### Namespace_BindNativeFunctions  

`ani_status (*Namespace_BindNativeFunctions)(ani_env *env, ani_namespace ns, const ani_native_function *functions, ani_size nr_functions);`  
**PARAMETERS:**
env: A pointer to the environment structure.
ns: The namespace to which the native functions will be bound.
functions: A pointer to an array of native functions to bind.
nr_functions: The number of native functions in the array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Boolean  

`ani_status (*Function_Call_Boolean)(ani_env *env, ani_function fn, ani_boolean *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the boolean result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Boolean_A  

`ani_status (*Function_Call_Boolean_A)(ani_env *env, ani_function fn, ani_boolean *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the boolean result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Boolean_V  

`ani_status (*Function_Call_Boolean_V)(ani_env *env, ani_function fn, ani_boolean *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the boolean result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Char  

`ani_status (*Function_Call_Char)(ani_env *env, ani_function fn, ani_char *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the character result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Char_A  

`ani_status (*Function_Call_Char_A)(ani_env *env, ani_function fn, ani_char *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the character result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Char_V  

`ani_status (*Function_Call_Char_V)(ani_env *env, ani_function fn, ani_char *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the character result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Byte  

`ani_status (*Function_Call_Byte)(ani_env *env, ani_function fn, ani_byte *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the byte result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Byte_A  

`ani_status (*Function_Call_Byte_A)(ani_env *env, ani_function fn, ani_byte *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the byte result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Byte_V  

`ani_status (*Function_Call_Byte_V)(ani_env *env, ani_function fn, ani_byte *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the byte result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Short  

`ani_status (*Function_Call_Short)(ani_env *env, ani_function fn, ani_short *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the short result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Short_A  

`ani_status (*Function_Call_Short_A)(ani_env *env, ani_function fn, ani_short *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the short result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Short_V  

`ani_status (*Function_Call_Short_V)(ani_env *env, ani_function fn, ani_short *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the short result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Int  

`ani_status (*Function_Call_Int)(ani_env *env, ani_function fn, ani_int *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the integer result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Int_A

`ani_status (*Function_Call_Int_A)(ani_env *env, ani_function fn, ani_int *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the integer result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Int_V

`ani_status (*Function_Call_Int_V)(ani_env *env, ani_function fn, ani_int *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the integer result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Long  

`ani_status (*Function_Call_Long)(ani_env *env, ani_function fn, ani_long *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the long result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Long_A  

`ani_status (*Function_Call_Long_A)(ani_env *env, ani_function fn, ani_long *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the long result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Long_V  

`ani_status (*Function_Call_Long_V)(ani_env *env, ani_function fn, ani_long *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the long result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Float  

`ani_status (*Function_Call_Float)(ani_env *env, ani_function fn, ani_float *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the float result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Float_A  

`ani_status (*Function_Call_Float_A)(ani_env *env, ani_function fn, ani_float *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the float result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Float_V  

`ani_status (*Function_Call_Float_V)(ani_env *env, ani_function fn, ani_float *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the float result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Double  

`ani_status (*Function_Call_Double)(ani_env *env, ani_function fn, ani_double *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the double result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Double_A  

`ani_status (*Function_Call_Double_A)(ani_env *env, ani_function fn, ani_double *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the double result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Double_V  

`ani_status (*Function_Call_Double_V)(ani_env *env, ani_function fn, ani_double *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the double result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Ref  

`ani_status (*Function_Call_Ref)(ani_env *env, ani_function fn, ani_ref *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the reference result.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Ref_A  

`ani_status (*Function_Call_Ref_A)(ani_env *env, ani_function fn, ani_ref *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the reference result.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Ref_V  

`ani_status (*Function_Call_Ref_V)(ani_env *env, ani_function fn, ani_ref *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
result: A pointer to store the reference result.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Void  

`ani_status (*Function_Call_Void)(ani_env *env, ani_function fn, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
...: Variadic arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Void_A  

`ani_status (*Function_Call_Void_A)(ani_env *env, ani_function fn, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
args: A pointer to an array of arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Function_Call_Void_V  

`ani_status (*Function_Call_Void_V)(ani_env *env, ani_function fn, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
fn: The function to call.
args: A `va_list` containing the arguments to pass to the function.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Char  

`ani_status (*Object_CallMethod_Char)(ani_env *env, ani_object object, ani_method method, ani_char *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the char return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Char_A  

`ani_status (*Object_CallMethod_Char_A)(ani_env *env, ani_object object, ani_method method, ani_char *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the char return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Char_V  

`ani_status (*Object_CallMethod_Char_V)(ani_env *env, ani_object object, ani_method method, ani_char *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the char return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Byte  

`ani_status (*Object_CallMethod_Byte)(ani_env *env, ani_object object, ani_method method, ani_byte *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the byte return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Byte_A  

`ani_status (*Object_CallMethod_Byte_A)(ani_env *env, ani_object object, ani_method method, ani_byte *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the byte return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Byte_V  

`ani_status (*Object_CallMethod_Byte_V)(ani_env *env, ani_object object, ani_method method, ani_byte *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the byte return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Short  

`ani_status (*Object_CallMethod_Short)(ani_env *env, ani_object object, ani_method method, ani_short *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the short return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Short_A  

`ani_status (*Object_CallMethod_Short_A)(ani_env *env, ani_object object, ani_method method, ani_short *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the short return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Short_V  

`ani_status (*Object_CallMethod_Short_V)(ani_env *env, ani_object object, ani_method method, ani_short *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the short return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Float  

`ani_status (*Object_CallMethod_Float)(ani_env *env, ani_object object, ani_method method, ani_float *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the float return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Float_A  

`ani_status (*Object_CallMethod_Float_A)(ani_env *env, ani_object object, ani_method method, ani_float *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the float return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Float_V  

`ani_status (*Object_CallMethod_Float_V)(ani_env *env, ani_object object, ani_method method, ani_float *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the float return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Ref  

`ani_status (*Object_CallMethod_Ref)(ani_env *env, ani_object object, ani_method method, ani_ref *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the reference return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Ref_A  

`ani_status (*Object_CallMethod_Ref_A)(ani_env *env, ani_object object, ani_method method, ani_ref *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the reference return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Ref_V  

`ani_status (*Object_CallMethod_Ref_V)(ani_env *env, ani_object object, ani_method method, ani_ref *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the reference return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Boolean  

`ani_status (*Object_CallMethodByName_Boolean)(ani_env *env, ani_object object, const char *name, const char *signature, ani_boolean *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the boolean return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Boolean_A  

`ani_status (*Object_CallMethodByName_Boolean_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_boolean *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the boolean return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Boolean_V  

`ani_status (*Object_CallMethodByName_Boolean_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_boolean *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the boolean return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Char  

`ani_status (*Object_CallMethodByName_Char)(ani_env *env, ani_object object, const char *name, const char *signature, ani_char *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the char return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Char_A  

`ani_status (*Object_CallMethodByName_Char_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_char *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the char return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Char_V  

`ani_status (*Object_CallMethodByName_Char_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_char *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the char return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Byte  

`ani_status (*Object_CallMethodByName_Byte)(ani_env *env, ani_object object, const char *name, const char *signature, ani_byte *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the byte return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Byte_A  

`ani_status (*Object_CallMethodByName_Byte_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_byte *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the byte return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Byte_V  

`ani_status (*Object_CallMethodByName_Byte_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_byte *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the byte return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Short  

`ani_status (*Object_CallMethodByName_Short)(ani_env *env, ani_object object, const char *name, const char *signature, ani_short *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the short return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Short_A  

`ani_status (*Object_CallMethodByName_Short_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_short *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the short return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Short_V  

`ani_status (*Object_CallMethodByName_Short_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_short *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the short return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Int  

`ani_status (*Object_CallMethodByName_Int)(ani_env *env, ani_object object, const char *name, const char *signature, ani_int *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the integer return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Int_A  

`ani_status (*Object_CallMethodByName_Int_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_int *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the integer return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Int_V  

`ani_status (*Object_CallMethodByName_Int_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_int *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the integer return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Long  

`ani_status (*Object_CallMethodByName_Long)(ani_env *env, ani_object object, const char *name, const char *signature, ani_long *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the long return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Long_A  

`ani_status (*Object_CallMethodByName_Long_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_long *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the long return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Long_V  

`ani_status (*Object_CallMethodByName_Long_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_long *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the long return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Float  

`ani_status (*Object_CallMethodByName_Float)(ani_env *env, ani_object object, const char *name, const char *signature, ani_float *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the float return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Float_A  

`ani_status (*Object_CallMethodByName_Float_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_float *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the float return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Float_V  

`ani_status (*Object_CallMethodByName_Float_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_float *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the float return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Double  

`ani_status (*Object_CallMethodByName_Double)(ani_env *env, ani_object object, const char *name, const char *signature, ani_double *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the double return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Double_A  

`ani_status (*Object_CallMethodByName_Double_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_double *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the double return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Double_V  

`ani_status (*Object_CallMethodByName_Double_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_double *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the double return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Ref  

`ani_status (*Object_CallMethodByName_Ref)(ani_env *env, ani_object object, const char *name, const char *signature, ani_ref *result, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the reference return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Ref_A  

`ani_status (*Object_CallMethodByName_Ref_A)(ani_env *env, ani_object object, const char *name, const char *signature, ani_ref *result, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the reference return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Ref_V  

`ani_status (*Object_CallMethodByName_Ref_V)(ani_env *env, ani_object object, const char *name, const char *signature, ani_ref *result, va_list args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
result: A pointer to store the reference return value.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Void  

`ani_status (*Object_CallMethodByName_Void)(ani_env *env, ani_object object, const char *name, const char *signature, ...);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Void_A  

`ani_status (*Object_CallMethodByName_Void_A)(ani_env *env, ani_object object, const char *name, const char *signature, const ani_value *args);`  
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethodByName_Void_V  

`ani_status (*Object_CallMethodByName_Void_V)(ani_env *env, ani_object object, const char *name, const char *signature, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
name: The name of the method to call.
signature: The signature of the method to call.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Object_CallMethod_Void

`ani_status (*Object_CallMethod_Void)(ani_env *env, ani_object object, ani_method method, ...);`
This function calls the specified method of an object using variadic arguments. The method does not return a value.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
return the same status code as Object_CallMethod_Void_V

### Object_CallMethod_Void_A

`ani_status (*Object_CallMethod_Void_A)(ani_env *env, ani_object object, ani_method method, const ani_value *args);`
This function calls the specified method of an object using arguments provided in an array. The method does not return a value.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
args: an array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: objct == nullptr || method == nullptr || args == nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Void_V

`ani_status (*Object_CallMethod_Void_V)(ani_env *env, ani_object object, ani_method method, va_list args);`
This function calls the specified method of an object using a va_list. The method does not return a value.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
args: A va_list of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: objct == nullptr || method == nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Long

`ani_status (*Object_CallMethod_Long)(ani_env *env, ani_object object, ani_method method, ani_long *result, ...);`
This function calls the specified method of an object using variadic arguments and retrieves a long result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the long return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
Return ANI_OK : calls the specified method of an object using variadic arguments and retrieves a long result success
Return ANI_INVALID_ARGS : object == nullptr || method == nullptr || result == nullptr

### Object_CallMethod_Long_A

`ani_status (*Object_CallMethod_Long_A)(ani_env *env, ani_object object, ani_method method, ani_long *result, const ani_value *args);`
This function calls the specified method of an object using arguments provided in an array and retrieves a long result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the long return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
Return ANI_OK : calls the specified method of an object using variadic arguments and retrieves a long result success
Return ANI_INVALID_ARGS : object == nullptr || method == nullptr || result == nullptr || args == nullptr

### Object_CallMethod_Long_V

`ani_status (*Object_CallMethod_Long_V)(ani_env *env, ani_object object, ani_method method, ani_long *result, va_list args);`
This function calls the specified method of an object using a va_list and retrieves a long result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the long return value.
args: A va_list of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
Return ANI_OK : calls the specified method of an object using variadic arguments and retrieves a long result success
Return ANI_INVALID_ARGS : object == nullptr || method == nullptr || result == nullptr

### Object_CallMethod_Boolean

`ani_status (*Object_CallMethod_Boolean)(ani_env *env, ani_object object, ani_method method, ani_boolean *result, ...);`
This function calls the specified method of an object using variadic arguments and retrieves a boolean result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the boolean return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Boolean_A

`ani_status (*Object_CallMethod_Boolean_A)(ani_env *env, ani_object object, ani_method method, ani_boolean *result, const ani_value *args);`
This function calls the specified method of an object using arguments provided in an array and retrieves a boolean result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the boolean return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Boolean_V

`ani_status (*Object_CallMethod_Boolean_V)(ani_env *env, ani_object object, ani_method method, ani_boolean *result, va_list args);`
This function calls the specified method of an object using a va_list and retrieves a boolean result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the boolean return value.
args: A va_list of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || args == nullptr || result ==nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Byte

`ani_status (*Object_CallMethod_Byte)(ani_env *env, ani_object object, ani_method method, ani_byte *result, ...);`
This function calls the specified method of an object using variadic arguments and retrieves a byte result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the byte return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Byte_A

`ani_status (*Object_CallMethod_Byte_A)(ani_env *env, ani_object object, ani_method method, ani_byte *result, const ani_value *args);`
This function calls the specified method of an object using arguments provided in an array and retrieves a byte result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the byte return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr || args == nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Byte_V

`ani_status (*Object_CallMethod_Byte_V)(ani_env *env, ani_object object, ani_method method, ani_byte *result, va_list args);`
This function calls the specified method of an object using a va_list and retrieves a byte result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the byte return value.
args: A va_list of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result ==nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Short

`ani_status (*Object_CallMethod_Short)(ani_env *env, ani_object object, ani_method method, ani_short *result, ...);`
This function calls the specified method of an object using variadic arguments and retrieves a short result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the short return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Short_A

`ani_status (*Object_CallMethod_Short_A)(ani_env *env, ani_object object, ani_method method, ani_short *result, const ani_value *args);`
This function calls the specified method of an object using arguments provided in an array and retrieves a short result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the short return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr || args == nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Short_V

`ani_status (*Object_CallMethod_Short_V)(ani_env *env, ani_object object, ani_method method, ani_short *result, va_list args);`
This function calls the specified method of an object using a va_list and retrieves a short result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the short return value.
args: A va_list of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result ==nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Float

`ani_status (*Object_CallMethod_Float)(ani_env *env, ani_object object, ani_method method, ani_float *result, ...);`
This function calls the specified method of an object using variadic arguments and retrieves a float result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the float return value.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Float_A

`ani_status (*Object_CallMethod_Float_A)(ani_env *env, ani_object object, ani_method method, ani_float *result, const ani_value *args);`
This function calls the specified method of an object using arguments provided in an array and retrieves a float result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the float return value.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result== nullptr || args == nullptr
ANI_OK : calling mehtod success

### Object_CallMethod_Float_V

`ani_status (*Object_CallMethod_Float_V)(ani_env *env, ani_object object, ani_method method, ani_float *result, va_list args);`
This function calls the specified method of an object using a va_list and retrieves a float result.
**PARAMETERS:**
env: A pointer to the environment structure.
object: The object on which the method is to be called.
method: The method to call.
result: A pointer to store the float return value.
args: A va_list of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: object== nullptr || method== nullptr || result ==nullptr
ANI_OK : calling mehtod success

## 9. Accessing Static Fields

### Class_GetStaticField_Boolean

`ani_status (*Class_GetStaticField_Boolean)(ani_env *env, ani_class cls, ani_static_field field, ani_boolean *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved boolean value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_GetStaticField_Char

`ani_status (*Class_GetStaticField_Char)(ani_env *env, ani_class cls, ani_static_field field, ani_char *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved character value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticField_Byte

`ani_status (*Class_GetStaticField_Byte)(ani_env *env, ani_class cls, ani_static_field field, ani_byte *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved byte value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticField_Short

`ani_status (*Class_GetStaticField_Short)(ani_env *env, ani_class cls, ani_static_field field, ani_short *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved short value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticField_Int

`ani_status (*Class_GetStaticField_Int)(ani_env *env, ani_class cls, ani_static_field field, ani_int *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved integer value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticField_Long

`ani_status (*Class_GetStaticField_Long)(ani_env *env, ani_class cls, ani_static_field field, ani_long *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved long value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticField_Float

`ani_status (*Class_GetStaticField_Float)(ani_env *env, ani_class cls, ani_static_field field, ani_float *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved float value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticField_Double

`ani_status (*Class_GetStaticField_Double)(ani_env *env, ani_class cls, ani_static_field field, ani_double *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved double value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticField_Ref

`ani_status (*Class_GetStaticField_Ref)(ani_env *env, ani_class cls, ani_static_field field, ani_ref *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to retrieve.
result: A pointer to store the retrieved reference value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Boolean

`ani_status (*Class_SetStaticField_Boolean)(ani_env *env, ani_class cls, ani_static_field field, ani_boolean value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The boolean value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Char

`ani_status (*Class_SetStaticField_Char)(ani_env *env, ani_class cls, ani_static_field field, ani_char value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The character value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Byte

`ani_status (*Class_SetStaticField_Byte)(ani_env *env, ani_class cls, ani_static_field field, ani_byte value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The byte value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Short

`ani_status (*Class_SetStaticField_Short)(ani_env *env, ani_class cls, ani_static_field field, ani_short value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The short value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Int

`ani_status (*Class_SetStaticField_Int)(ani_env *env, ani_class cls, ani_static_field field, ani_int value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The integer value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Long

`ani_status (*Class_SetStaticField_Long)(ani_env *env, ani_class cls, ani_static_field field, ani_long value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The long value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Float

`ani_status (*Class_SetStaticField_Float)(ani_env *env, ani_class cls, ani_static_field field, ani_float value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The float value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Double

`ani_status (*Class_SetStaticField_Double)(ani_env *env, ani_class cls, ani_static_field field, ani_double value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The double value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticField_Ref

`ani_status (*Class_SetStaticField_Ref)(ani_env *env, ani_class cls, ani_static_field field, ani_ref value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
field: The static field to modify.
value: The reference value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Boolean

`ani_status (*Class_GetStaticFieldByName_Boolean)(ani_env *env, ani_class cls, const char *name, ani_boolean *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved boolean value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Char

`ani_status (*Class_GetStaticFieldByName_Char)(ani_env *env, ani_class cls, const char *name, ani_char *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved character value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Byte

`ani_status (*Class_GetStaticFieldByName_Byte)(ani_env *env, ani_class cls, const char *name, ani_byte *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved byte value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Short

`ani_status (*Class_GetStaticFieldByName_Short)(ani_env *env, ani_class cls, const char *name, ani_short *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved short value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Int

`ani_status (*Class_GetStaticFieldByName_Int)(ani_env *env, ani_class cls, const char *name, ani_int *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved integer value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Long

`ani_status (*Class_GetStaticFieldByName_Long)(ani_env *env, ani_class cls, const char *name, ani_long *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved long value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Float

`ani_status (*Class_GetStaticFieldByName_Float)(ani_env *env, ani_class cls, const char *name, ani_float *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved float value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Double

`ani_status (*Class_GetStaticFieldByName_Double)(ani_env *env, ani_class cls, const char *name, ani_double *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved double value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_GetStaticFieldByName_Ref

`ani_status (*Class_GetStaticFieldByName_Ref)(ani_env *env, ani_class cls, const char *name, ani_ref *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to retrieve.
result: A pointer to store the retrieved reference value.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Boolean

`ani_status (*Class_SetStaticFieldByName_Boolean)(ani_env *env, ani_class cls, const char *name, ani_boolean value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The boolean value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Char

`ani_status (*Class_SetStaticFieldByName_Char)(ani_env *env, ani_class cls, const char *name, ani_char value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The character value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Byte

`ani_status (*Class_SetStaticFieldByName_Byte)(ani_env *env, ani_class cls, const char *name, ani_byte value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The byte value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Short

`ani_status (*Class_SetStaticFieldByName_Short)(ani_env *env, ani_class cls, const char *name, ani_short value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The short value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Int

`ani_status (*Class_SetStaticFieldByName_Int)(ani_env *env, ani_class cls, const char *name, ani_int value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The integer value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Long

`ani_status (*Class_SetStaticFieldByName_Long)(ani_env *env, ani_class cls, const char *name, ani_long value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The long value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Float

`ani_status (*Class_SetStaticFieldByName_Float)(ani_env *env, ani_class cls, const char *name, ani_float value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The float value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Double

`ani_status (*Class_SetStaticFieldByName_Double)(ani_env *env, ani_class cls, const char *name, ani_double value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The double value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Ref

`ani_status (*Class_SetStaticFieldByName_Ref)(ani_env *env, ani_class cls, const char *name, ani_ref value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The reference value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
## 10. Calling Static Methods

### Class_CallStaticMethod_Ref

`ani_status (*Class_CallStaticMethod_Ref)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the reference result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Ref_V

`ani_status (*Class_CallStaticMethod_Ref_V)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the reference result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Ref_A

`ani_status (*Class_CallStaticMethod_Ref_A)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the reference result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Boolean

`ani_status (*Class_CallStaticMethod_Boolean)(ani_env *env, ani_class cls, ani_static_method method, ani_boolean *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the boolean result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Boolean_A

`ani_status (*Class_CallStaticMethod_Boolean_A)(ani_env *env, ani_class cls, ani_static_method method, ani_boolean *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the boolean result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Boolean_V

`ani_status (*Class_CallStaticMethod_Boolean_V)(ani_env *env, ani_class cls, ani_static_method method, ani_boolean *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the boolean result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Char

`ani_status (*Class_CallStaticMethod_Char)(ani_env *env, ani_class cls, ani_static_method method, ani_char *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the character result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Char_A

`ani_status (*Class_CallStaticMethod_Char_A)(ani_env *env, ani_class cls, ani_static_method method, ani_char *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the character result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Char_V

`ani_status (*Class_CallStaticMethod_Char_V)(ani_env *env, ani_class cls, ani_static_method method, ani_char *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the character result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Byte

`ani_status (*Class_CallStaticMethod_Byte)(ani_env *env, ani_class cls, ani_static_method method, ani_byte *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the byte result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Byte_A

`ani_status (*Class_CallStaticMethod_Byte_A)(ani_env *env, ani_class cls, ani_static_method method, ani_byte *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the byte result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Byte_V

`ani_status (*Class_CallStaticMethod_Byte_V)(ani_env *env, ani_class cls, ani_static_method method, ani_byte *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the byte result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Short

`ani_status (*Class_CallStaticMethod_Short)(ani_env *env, ani_class cls, ani_static_method method, ani_short *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the short result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Short_A

`ani_status (*Class_CallStaticMethod_Short_A)(ani_env *env, ani_class cls, ani_static_method method, ani_short *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the short result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Short_V

`ani_status (*Class_CallStaticMethod_Short_V)(ani_env *env, ani_class cls, ani_static_method method, ani_short *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the short result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Int

`ani_status (*Class_CallStaticMethod_Int)(ani_env *env, ani_class cls, ani_static_method method, ani_int *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the integer result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Int_A

`ani_status (*Class_CallStaticMethod_Int_A)(ani_env *env, ani_class cls, ani_static_method method, ani_int *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the integer result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Int_V

`ani_status (*Class_CallStaticMethod_Int_V)(ani_env *env, ani_class cls, ani_static_method method, ani_int *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the integer result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Long

`ani_status (*Class_CallStaticMethod_Long)(ani_env *env, ani_class cls, ani_static_method method, ani_long *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the long result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Long_A

`ani_status (*Class_CallStaticMethod_Long_A)(ani_env *env, ani_class cls, ani_static_method method, ani_long *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the long result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Long_V

`ani_status (*Class_CallStaticMethod_Long_V)(ani_env *env, ani_class cls, ani_static_method method, ani_long *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the long result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Float

`ani_status (*Class_CallStaticMethod_Float)(ani_env *env, ani_class cls, ani_static_method method, ani_float *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the float result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Float_A

`ani_status (*Class_CallStaticMethod_Float_A)(ani_env *env, ani_class cls, ani_static_method method, ani_float *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the float result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Float_V

`ani_status (*Class_CallStaticMethod_Float_V)(ani_env *env, ani_class cls, ani_static_method method, ani_float *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the float result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Double

`ani_status (*Class_CallStaticMethod_Double)(ani_env *env, ani_class cls, ani_static_method method, ani_double *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the double result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Double_A

`ani_status (*Class_CallStaticMethod_Double_A)(ani_env *env, ani_class cls, ani_static_method method, ani_double *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the double result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Double_V

`ani_status (*Class_CallStaticMethod_Double_V)(ani_env *env, ani_class cls, ani_static_method method, ani_double *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the double result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Void

`ani_status (*Class_CallStaticMethod_Void)(ani_env *env, ani_class cls, ani_static_method method, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Void_A

`ani_status (*Class_CallStaticMethod_Void_A)(ani_env *env, ani_class cls, ani_static_method method, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Void_V

`ani_status (*Class_CallStaticMethod_Void_V)(ani_env *env, ani_class cls, ani_static_method method, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Boolean

`ani_status (*Class_CallStaticMethodByName_Boolean)(ani_env *env, ani_class cls, const char *name, ani_boolean *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the boolean result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Boolean_A

`ani_status (*Class_CallStaticMethodByName_Boolean_A)(ani_env *env, ani_class cls, const char *name, ani_boolean *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the boolean result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Boolean_V

`ani_status (*Class_CallStaticMethodByName_Boolean_V)(ani_env *env, ani_class cls, const char *name, ani_boolean *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the boolean result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Char

`ani_status (*Class_CallStaticMethodByName_Char)(ani_env *env, ani_class cls, const char *name, ani_char *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the char result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Char_A

`ani_status (*Class_CallStaticMethodByName_Char_A)(ani_env *env, ani_class cls, const char *name, ani_char *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the char result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Char_V

`ani_status (*Class_CallStaticMethodByName_Char_V)(ani_env *env, ani_class cls, const char *name, ani_char *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the char result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Byte

`ani_status (*Class_CallStaticMethodByName_Byte)(ani_env *env, ani_class cls, const char *name, ani_byte *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the byte result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Byte_A

`ani_status (*Class_CallStaticMethodByName_Byte_A)(ani_env *env, ani_class cls, const char *name, ani_byte *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the byte result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Byte_V

`ani_status (*Class_CallStaticMethodByName_Byte_V)(ani_env *env, ani_class cls, const char *name, ani_byte *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the byte result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Short

`ani_status (*Class_CallStaticMethodByName_Short)(ani_env *env, ani_class cls, const char *name, ani_short *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the short result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Short_A

`ani_status (*Class_CallStaticMethodByName_Short_A)(ani_env *env, ani_class cls, const char *name, ani_short *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the short result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Short_V

`ani_status (*Class_CallStaticMethodByName_Short_V)(ani_env *env, ani_class cls, const char *name, ani_short *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the short result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Int

`ani_status (*Class_CallStaticMethodByName_Int)(ani_env *env, ani_class cls, const char *name, ani_int *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the integer result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Int_A

`ani_status (*Class_CallStaticMethodByName_Int_A)(ani_env *env, ani_class cls, const char *name, ani_int *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the integer result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Int_V

`ani_status (*Class_CallStaticMethodByName_Int_V)(ani_env *env, ani_class cls, const char *name, ani_int *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the integer result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Long

`ani_status (*Class_CallStaticMethodByName_Long)(ani_env *env, ani_class cls, const char *name, ani_long *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the long result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Long_A

`ani_status (*Class_CallStaticMethodByName_Long_A)(ani_env *env, ani_class cls, const char *name, ani_long *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the long result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Long_V

`ani_status (*Class_CallStaticMethodByName_Long_V)(ani_env *env, ani_class cls, const char *name, ani_long *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the long result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Float

`ani_status (*Class_CallStaticMethodByName_Float)(ani_env *env, ani_class cls, const char *name, ani_float *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the float result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Float_A

`ani_status (*Class_CallStaticMethodByName_Float_A)(ani_env *env, ani_class cls, const char *name, ani_float *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the float result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Float_V

`ani_status (*Class_CallStaticMethodByName_Float_V)(ani_env *env, ani_class cls, const char *name, ani_float *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the float result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Double

`ani_status (*Class_CallStaticMethodByName_Double)(ani_env *env, ani_class cls, const char *name, ani_double *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the double result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Double_A

`ani_status (*Class_CallStaticMethodByName_Double_A)(ani_env *env, ani_class cls, const char *name, ani_double *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the double result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Double_V

`ani_status (*Class_CallStaticMethodByName_Double_V)(ani_env *env, ani_class cls, const char *name, ani_double *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the double result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Ref

`ani_status (*Class_CallStaticMethodByName_Ref)(ani_env *env, ani_class cls, const char *name, ani_ref *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the reference result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Ref_A

`ani_status (*Class_CallStaticMethodByName_Ref_A)(ani_env *env, ani_class cls, const char *name, ani_ref *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the reference result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Ref_V

`ani_status (*Class_CallStaticMethodByName_Ref_V)(ani_env *env, ani_class cls, const char *name, ani_ref *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the reference result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Void

`ani_status (*Class_CallStaticMethodByName_Void)(ani_env *env, ani_class cls, const char *name, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Void_A

`ani_status (*Class_CallStaticMethodByName_Void_A)(ani_env *env, ani_class cls, const char *name, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The classClass_CallStaticMethodByName_Void_A containing the static method.
name: The name of the static method to call.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Void_V

`ani_status (*Class_CallStaticMethodByName_Void_V)(ani_env *env, ani_class cls, const char *name, va_list args);`
**PARAMETERS:**
 env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Ref

`ani_status (*Class_CallStaticMethod_Ref)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the reference result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Ref_V

`ani_status (*Class_CallStaticMethod_Ref_V)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the reference result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Ref_A

`ani_status (*Class_CallStaticMethod_Ref_A)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the reference result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Boolean

`ani_status (*Class_CallStaticMethod_Boolean)(ani_env *env, ani_class cls, ani_static_method method, ani_boolean *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the boolean result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Boolean_A

`ani_status (*Class_CallStaticMethod_Boolean_A)(ani_env *env, ani_class cls, ani_static_method method, ani_boolean *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the boolean result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Boolean_V

`ani_status (*Class_CallStaticMethod_Boolean_V)(ani_env *env, ani_class cls, ani_static_method method, ani_boolean *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the boolean result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Char

`ani_status (*Class_CallStaticMethod_Char)(ani_env *env, ani_class cls, ani_static_method method, ani_char *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the character result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Char_A

`ani_status (*Class_CallStaticMethod_Char_A)(ani_env *env, ani_class cls, ani_static_method method, ani_char *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the character result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Char_V

`ani_status (*Class_CallStaticMethod_Char_V)(ani_env *env, ani_class cls, ani_static_method method, ani_char *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the character result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Byte

`ani_status (*Class_CallStaticMethod_Byte)(ani_env *env, ani_class cls, ani_static_method method, ani_byte *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the byte result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Byte_A

`ani_status (*Class_CallStaticMethod_Byte_A)(ani_env *env, ani_class cls, ani_static_method method, ani_byte *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the byte result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Byte_V

`ani_status (*Class_CallStaticMethod_Byte_V)(ani_env *env, ani_class cls, ani_static_method method, ani_byte *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the byte result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Short

`ani_status (*Class_CallStaticMethod_Short)(ani_env *env, ani_class cls, ani_static_method method, ani_short *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the short result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Short_A

`ani_status (*Class_CallStaticMethod_Short_A)(ani_env *env, ani_class cls, ani_static_method method, ani_short *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the short result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Short_V

`ani_status (*Class_CallStaticMethod_Short_V)(ani_env *env, ani_class cls, ani_static_method method, ani_short *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the short result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Int

`ani_status (*Class_CallStaticMethod_Int)(ani_env *env, ani_class cls, ani_static_method method, ani_int *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the integer result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Int_A

`ani_status (*Class_CallStaticMethod_Int_A)(ani_env *env, ani_class cls, ani_static_method method, ani_int *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the integer result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Int_V

`ani_status (*Class_CallStaticMethod_Int_V)(ani_env *env, ani_class cls, ani_static_method method, ani_int *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the integer result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Long

`ani_status (*Class_CallStaticMethod_Long)(ani_env *env, ani_class cls, ani_static_method method,  ani_long *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the long result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Long_A

`ani_status (*Class_CallStaticMethod_Long_A)(ani_env *env, ani_class cls, ani_static_method method, ani_long *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the long result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Long_V

`ani_status (*Class_CallStaticMethod_Long_V)(ani_env *env, ani_class cls, ani_static_method method, ani_long *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the long result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Float

`ani_status (*Class_CallStaticMethod_Float)(ani_env *env, ani_class cls, ani_static_method method, ani_float *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the float result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Float_A

`ani_status (*Class_CallStaticMethod_Float_A)(ani_env *env, ani_class cls, ani_static_method method, ani_float *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the float result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Float_V

`ani_status (*Class_CallStaticMethod_Float_V)(ani_env *env, ani_class cls, ani_static_method method, ani_float *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the float result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Double

`ani_status (*Class_CallStaticMethod_Double)(ani_env *env, ani_class cls, ani_static_method method,  ani_double *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the double result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Double_A

`ani_status (*Class_CallStaticMethod_Double_A)(ani_env *env, ani_class cls, ani_static_method method, ani_double *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the double result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Double_V

`ani_status (*Class_CallStaticMethod_Double_V)(ani_env *env, ani_class cls, ani_static_method method, ani_double *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
result: A pointer to store the double result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Void

`ani_status (*Class_CallStaticMethod_Void)(ani_env *env, ani_class cls, ani_static_method method, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Void_A

`ani_status (*Class_CallStaticMethod_Void_A)(ani_env *env, ani_class cls, ani_static_method method, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Void_V

`ani_status (*Class_CallStaticMethod_Void_V)(ani_env *env, ani_class cls, ani_static_method method, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
method: The static method to call.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Boolean

`ani_status (*Class_CallStaticMethodByName_Boolean)(ani_env *env, ani_class cls, const char *name, ani_boolean *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the boolean result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Boolean_A

`ani_status (*Class_CallStaticMethodByName_Boolean_A)(ani_env *env, ani_class cls, const char *name, ani_boolean *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the boolean result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Boolean_V

`ani_status (*Class_CallStaticMethodByName_Boolean_V)(ani_env *env, ani_class cls, const char *name, ani_boolean *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the boolean result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Char

`ani_status (*Class_CallStaticMethodByName_Char)(ani_env *env, ani_class cls, const char *name, ani_char *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the char result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Char_A

`ani_status (*Class_CallStaticMethodByName_Char_A)(ani_env *env, ani_class cls, const char *name, ani_char *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the char result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Char_V

`ani_status (*Class_CallStaticMethodByName_Char_V)(ani_env *env, ani_class cls, const char *name, ani_char *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the char result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Byte

`ani_status (*Class_CallStaticMethodByName_Byte)(ani_env *env, ani_class cls, const char *name, ani_byte *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the byte result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Byte_A

`ani_status (*Class_CallStaticMethodByName_Byte_A)(ani_env *env, ani_class cls, const char *name, ani_byte *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the byte result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Byte_V

`ani_status (*Class_CallStaticMethodByName_Byte_V)(ani_env *env, ani_class cls, const char *name, ani_byte *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the byte result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Short

`ani_status (*Class_CallStaticMethodByName_Short)(ani_env *env, ani_class cls, const char *name, ani_short *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the short result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Short_A

`ani_status (*Class_CallStaticMethodByName_Short_A)(ani_env *env, ani_class cls, const char *name, ani_short *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the short result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Short_V

`ani_status (*Class_CallStaticMethodByName_Short_V)(ani_env *env, ani_class cls, const char *name, ani_short *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the short result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Int

`ani_status (*Class_CallStaticMethodByName_Int)(ani_env *env, ani_class cls, const char *name, ani_int *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the integer result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Int_A

`ani_status (*Class_CallStaticMethodByName_Int_A)(ani_env *env, ani_class cls, const char *name, ani_int *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the integer result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Int_V

`ani_status (*Class_CallStaticMethodByName_Int_V)(ani_env *env, ani_class cls, const char *name, ani_int *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the integer result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Long

`ani_status (*Class_CallStaticMethodByName_Long)(ani_env *env, ani_class cls, const char *name,  ani_long *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the long result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Long_A

`ani_status (*Class_CallStaticMethodByName_Long_A)(ani_env *env, ani_class cls, const char *name, ani_long *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the long result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Long_V

`ani_status (*Class_CallStaticMethodByName_Long_V)(ani_env *env, ani_class cls, const char *name, ani_long *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the long result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Float

`ani_status (*Class_CallStaticMethodByName_Float)(ani_env *env, ani_class cls, const char *name, ani_float *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the float result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Float_A

`ani_status (*Class_CallStaticMethodByName_Float_A)(ani_env *env, ani_class cls, const char *name, ani_float *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the float result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Float_V

`ani_status (*Class_CallStaticMethodByName_Float_V)(ani_env *env, ani_class cls, const char *name, ani_float *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the float result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Double

`ani_status (*Class_CallStaticMethodByName_Double)(ani_env *env, ani_class cls, const char *name,  ani_double *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the double result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Double_A

`ani_status (*Class_CallStaticMethodByName_Double_A)(ani_env *env, ani_class cls, const char *name, ani_double *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the double result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Double_V

`ani_status (*Class_CallStaticMethodByName_Double_V)(ani_env *env, ani_class cls, const char *name, ani_double *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the double result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Ref

`ani_status (*Class_CallStaticMethodByName_Ref)(ani_env *env, ani_class cls, const char *name, ani_ref *result, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the reference result.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Ref_A

`ani_status (*Class_CallStaticMethodByName_Ref_A)(ani_env *env, ani_class cls, const char *name, ani_ref *result, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the reference result.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Ref_V

`ani_status (*Class_CallStaticMethodByName_Ref_V)(ani_env *env, ani_class cls, const char *name, ani_ref *result, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
result: A pointer to store the reference result.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Void

`ani_status (*Class_CallStaticMethodByName_Void)(ani_env *env, ani_class cls, const char *name, ...);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
...: Variadic arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Void_A

`ani_status (*Class_CallStaticMethodByName_Void_A)(ani_env *env, ani_class cls, const char *name, const ani_value *args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The classClass_CallStaticMethodByName_Void_A containing the static method.
name: The name of the static method to call.
args: An array of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethodByName_Void_V

`ani_status (*Class_CallStaticMethodByName_Void_V)(ani_env *env, ani_class cls, const char *name, va_list args);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static method.
name: The name of the static method to call.
args: A `va_list` of arguments to pass to the method.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_CallStaticMethod_Ref

`ani_status (*Class_CallStaticMethod_Ref)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, ...);`
This function can obtain the reference of the return value of the static method based on the class and its static method. This function can accept variable parameters.
**PARAMETERS:**
env: a pointer to the environment structure.
cls: a pointer to the static class.
method: a pointer to a static method.
result: a pointer to the result of the method call.
...: variable parameters.
**RETURNS:**
Returns: The working status of the function, indicating that the function is successfully executed or fails to be executed.
Return ANI_OK : get a reference to the return value of a method.
Return ANI_INVALID_ARGS : cls == nullptr || method == nullptr || result == nullptr

### Class_CallStaticMethod_Ref_V

`ani_status (*Class_CallStaticMethod_Ref_V)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, va_list args);`
This function can obtain the reference of the return value of the static method based on the class and its static method. This function can accept variable parameters.
**PARAMETERS:**
env: a pointer to the environment structure.
cls: a pointer to the static class.
method: a pointer to a static method.
result: a pointer to the result of the method call.
args: a pointer to the list of the input parameters of the method.
**RETURNS:**
Returns: The working status of the function, indicating that the function is successfully executed or fails to be executed.
Return ANI_OK : get a reference to the return value of a method.
Return ANI_INVALID_ARGS : cls == nullptr || method == nullptr || result == nullptr

### Class_CallStaticMethod_Ref_A

`ani_status (*Class_CallStaticMethod_Ref_A)(ani_env *env, ani_class cls, ani_static_method method, ani_ref *result, const ani_value *args);`
This function can obtain the reference of the return value of the static method based on the class and its static method.
**PARAMETERS:**
env: a pointer to the environment structure.
cls: a pointer to the static class.
method: a pointer to a static method.
result: a pointer to the result of the method call.
args: a pointer to the array of the input parameters of the method.
**RETURNS:**
Returns: The working status of the function, indicating that the function is successfully executed or fails to be executed.
Return ANI_OK : get a reference to the return value of a method.
Return ANI_INVALID_ARGS : cls == nullptr || method == nullptr || result == nullptr

##  11.String Operations

### String_GetUTF8Size

`ani_status (*String_GetUTF8Size)(ani_env *env, ani_string string, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
string: The UTF-8 string to measure.
result: A pointer to store the size of the string.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_GetUTF8SubString

`ani_status (*String_GetUTF8SubString)(ani_env *env, ani_string string, ani_size substr_offset, ani_size substr_size, char *utf_buffer, ani_size utf_buffer_size, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
string: The string to retrieve data from.
substr_offset: The starting offset of the substring.
substr_size: The size of the substring in bytes.
utf8_buffer: A buffer to store the substring.
utf8_buffer_size: The size of the buffer in bytes.
result: A pointer to store the number of bytes written.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_GetUTF16Size

`ani_status (*String_GetUTF16Size)(ani_env *env, ani_string string, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
string: The UTF-16 string to measure.
result: A pointer to store the size of the string.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_NewUTF8

`ani_status (*String_NewUTF8)(ani_env *env, const char *utf8_string, ani_size utf8_size, ani_string *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
utf8_string: A pointer to the UTF-8 encoded string data.
utf8_size: The size of the UTF-8 string in bytes.
result: A pointer to store the created string.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_GetUTF8

`ani_status (*String_GetUTF8)(ani_env *env, ani_string string, char *utf8_buffer, ani_size utf8_buffer_size, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
string: The string to retrieve data from.
utf8_buffer: A buffer to store the UTF-8 encoded data.
utf8_buffer_size: The size of the buffer in bytes.
result: A pointer to store the number of bytes written.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_NewUTF16

`ani_status (*String_NewUTF16)(ani_env *env, const uint16_t *utf16_string, ani_size utf16_size, ani_string *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
utf16_string: A pointer to the UTF-16 encoded string data.
utf16_size: The size of the UTF-16 string in code units.
result: A pointer to store the created string.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_GetUTF16

`ani_status (*String_GetUTF16)(ani_env *env, ani_string string, uint16_t *utf16_buffer, ani_size utf16_buffer_size, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
string: The string to retrieve data from.
utf16_buffer: A buffer to store the UTF-16 encoded data.
utf16_buffer_size: The size of the buffer in code units.
result: A pointer to store the number of code units written.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_GetUTF16SubString

`ani_status (*String_GetUTF16SubString)(ani_env *env, ani_string string, ani_size substr_offset, ani_size substr_size, uint16_t *utf16_buffer, ani_size utf16_buffer_size, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
string: The string to retrieve data from.
substr_offset: The starting offset of the substring.
substr_size: The size of the substring in code units.
utf16_buffer: A buffer to store the substring.
utf16_buffer_size: The size of the buffer in code units.
result: A pointer to store the number of code units written.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Long

`ani_status (*Class_SetStaticFieldByName_Long)(ani_env *env, ani_class cls, const char *name, ani_long value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The long value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Class_SetStaticFieldByName_Float

`ani_status (*Class_SetStaticFieldByName_Float)(ani_env *env, ani_class cls, const char *name, ani_float value);`
**PARAMETERS:**
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The float value to assign.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### String_GetUTF8Size

`ani_status (*String_GetUTF8Size)(ani_env *env, ani_string string, ani_size *result);`
This function retrieves the size(in bytes) of the specified UTF-8 string.(\0 is not included)
**PARAMETERS:**
env: A pointer to the environment structure.
string: The UTF-8 string to measure.
result: A pointer to store the size of the string.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: string == nullptr || result == nullptr
ANI_OK：get utf8 size success.

### String_GetUTF8SubString

`ani_status (*String_GetUTF8SubString)(ani_env *env, ani_string string, ani_size substr_offset, ani_size substr_size, char *utf_buffer, ani_size utf_buffer_size, ani_size *result);`
This function copies a portion of the UTF-8 string into the provided buffer.
**PARAMETERS:**
env: A pointer to the environment structure.
string:The string to retrieve data from.
substr_offset: The starting offset of the substring. e.g "emoji"
substr_size: The size of the substring in bytes.
utf8_buffer: A buffer to store the substring.
utf8_buffer_size: The size of the buffer in bytes.
result: A pointer to store the number of bytes written.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: string == nullptr || utf_buffer == bullptr || result == nullptr
ANI_BUFFER_TO_SMALL: (utf_buffer_size < substr_size) || (utf_buffer_size - substr_size) < 1
ANI_OUT_OF_RANGE: substr__offset > string->GetUTF8Length() || substr_size > (string->GetUTF8Length() - substr_offset)
ANI_OK: get substring success

### String_GetUTF16Size

`ani_status (*String_GetUTF16Size)(ani_env *env, ani_string string, ani_size *result);`
This function retrieves the size (in code units) of the specified UTF-16 string.
**PARAMETERS:**
env: A pointer to the environment structure.
string: The UTF-16 string to measure.
result: A pointer to store the size of the string.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: string == nullptr || result == nullptr
ANI_OK：get utf8 size success.

### String_NewUTF8

`ani_status (*String_NewUTF8)(ani_env *env, const char *utf8_string, ani_size size, ani_string *result);`
This function creates a new string from the provided UTF-8 encoded data.
**PARAMETERS:**
env: A pointer to the environment structure.
utf8_string: A pointer to the UTF-8 encoded string data.
utf8_size: The size of the UTF-8 string in bytes.
result: A pointer to store the created string.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_INVALID_ARGS: string == nullptr || result == nullptr
ANI_OUT_OF_MEMORY: can not create a  stirng from utf8_string
ANI_OK：get utf8 size success.

## 12. Array Operations

### FixedArray_Unpin

### Array_SetRegion_Byte

`ani_status (*Array_SetRegion_Byte)(ani_env *env, ani_fixedarray_byte array, ani_size offset, ani_size length, const ani_byte *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the byte values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Byte

`ani_status (*Array_GetRegion_Byte)(ani_env *env, ani_fixedarray_byte array, ani_size offset, ani_size length, ani_byte *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved byte values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### CreateArrayBuffer

`ani_status (*CreateArrayBuffer)(ani_env *env, size_t length, void **data_result, ani_arraybuffer *arraybuffer_result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array buffer in bytes.
data_result: A pointer to store the allocated data of the array buffer.
arraybuffer_result: A pointer to store the created array buffer object.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Boolean

`ani_status (*Array_New_Boolean)(ani_env *env, ani_size length, ani_fixedarray_boolean *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Char

`ani_status (*Array_New_Char)(ani_env *env, ani_size length, ani_fixedarray_char *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Byte

`ani_status (*Array_New_Byte)(ani_env *env, ani_size length, ani_fixedarray_byte *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Short

`ani_status (*Array_New_Short)(ani_env *env, ani_size length, ani_fixedarray_short *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Int

`ani_status (*Array_New_Int)(ani_env *env, ani_size length, ani_fixedarray_int *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Long

`ani_status (*Array_New_Long)(ani_env *env, ani_size length, ani_fixedarray_long *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Float

`ani_status (*Array_New_Float)(ani_env *env, ani_size length, ani_fixedarray_float *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Double

`ani_status (*Array_New_Double)(ani_env *env, ani_size length, ani_fixedarray_double *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_Set_Ref

`ani_status (*Array_Set_Ref)(ani_env *env, ani_fixedarray_ref array, ani_size index, ani_ref ref);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array of references to modify.
index: The index at which to set the reference.
ref: The reference to set at the specified index.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetLength

`ani_status (*Array_GetLength)(ani_env *env, ani_fixedarray array, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array whose length is to be retrieved.
result: A pointer to store the length of the array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_Get_Ref

`ani_status (*Array_Get_Ref)(ani_env *env, ani_fixedarray_ref array, ani_size index, ani_ref *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array of references to query.
index: The index from which to retrieve the reference.
result: A pointer to store the retrieved reference.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### ArrayBuffer_GetInfo

`ani_status (*ArrayBuffer_GetInfo)(ani_env *env, ani_arraybuffer arraybuffer, void **data_result, size_t *length_result);`
**PARAMETERS:**
env: A pointer to the environment structure.
arraybuffer: The array buffer to query.
data_result: A pointer to store the data of the array buffer.
length_result: A pointer to store the length of the array buffer in bytes.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Boolean

`ani_status (*Array_GetRegion_Boolean)(ani_env *env, ani_fixedarray_boolean array, ani_size offset, ani_size length, ani_boolean *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved boolean values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Char

`ani_status (*Array_GetRegion_Char)(ani_env *env, ani_fixedarray_char array, ani_size offset, ani_size length, ani_char *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved character values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Short

`ani_status (*Array_GetRegion_Short)(ani_env *env, ani_fixedarray_short array, ani_size offset, ani_size length, ani_short *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved short values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Int

`ani_status (*Array_GetRegion_Int)(ani_env *env, ani_fixedarray_int array, ani_size offset, ani_size length, ani_int *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved integer values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Long

`ani_status (*Array_GetRegion_Long)(ani_env *env, ani_fixedarray_long array, ani_size offset, ani_size length, ani_long *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved long integer values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Float

`ani_status (*Array_GetRegion_Float)(ani_env *env, ani_fixedarray_float array, ani_size offset, ani_size length, ani_float *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved float values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_GetRegion_Double

`ani_status (*Array_GetRegion_Double)(ani_env *env, ani_fixedarray_double array, ani_size offset, ani_size length, ani_double *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to retrieve values from.
offset: The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved double values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_SetRegion_Boolean

`ani_status (*Array_SetRegion_Boolean)(ani_env *env, ani_fixedarray_boolean array, ani_size offset, ani_size length, const ani_boolean *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the boolean values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_SetRegion_Char

`ani_status (*Array_SetRegion_Char)(ani_env *env, ani_fixedarray_char array, ani_size offset, ani_size length, const ani_char *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the character values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_SetRegion_Short

`ani_status (*Array_SetRegion_Short)(ani_env *env, ani_fixedarray_short array, ani_size offset, ani_size length, const ani_short *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the short values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_SetRegion_Int

`ani_status (*Array_SetRegion_Int)(ani_env *env, ani_fixedarray_int array, ani_size offset, ani_size length, const ani_int *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the integer values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_SetRegion_Long

`ani_status (*Array_SetRegion_Long)(ani_env *env, ani_fixedarray_long array, ani_size offset, ani_size length, const ani_long *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the long integer values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_SetRegion_Float

`ani_status (*Array_SetRegion_Float)(ani_env *env, ani_fixedarray_float array, ani_size offset, ani_size length, const ani_float *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the float values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_SetRegion_Double

`ani_status (*Array_SetRegion_Double)(ani_env *env, ani_fixedarray_double array, ani_size offset, ani_size length, const ani_double *native_buffer);`
**PARAMETERS:**
env: A pointer to the environment structure.
array: The fixed array to set values in.
offset: The starting offset of the region.
length: The number of elements to set.
native_buffer: A buffer containing the double values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### CreateArrayBufferExternal

`ani_status (*CreateArrayBufferExternal)(ani_env *env, void *external_data, size_t length, ani_finalizer finalizer, void *hint, ani_arraybuffer *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
external_data: A pointer to the external data to be used by the array buffer.
length: The length of the external data in bytes.
finalizer: A callback function to be called when the array buffer is finalized.
hint: A user-defined hint to be passed to the finalizer.
result: A pointer to store the created array buffer object.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.

### Array_New_Byte

`ani_status (*Array_New_Byte)(ani_env *env, ani_fixedarray_byte *result, ani_size length);`
This function creates a new fixed array of the specified length for byte values.
**PARAMETERS:**
env: A pointer to the environment structure.
length: The length of the array to be created.
result: A pointer to store the created fixed array.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_OK : creates a new fixed array of the specified length for byte values success
ANI_INVALID_ARGS: result == nullptr
ANI_OUT_OF_MEMORY: can not creates a new fixed array of the specified length for byte values

### Array_SetRegion_Byte

`ani_status (*Array_SetRegion_Byte)(ani_env *env, ani_fixedarray_byte array, ani_size offset, ani_size length, const ani_byte *native_buffer);`
This function sets a portion of the specified byte fixed array using a native buffer.
**PARAMETERS:**
env: A pointer to the environment structure.
array:The fixed array to set values in.
offset:The starting offset of the region.
length:The number of elements to set.
native_buffer:A buffer containing the byte values to set.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_OK : sets a portion of the specified byte fixed array using a native buffer success
ANI_INVALID_ARGS: array == nullptr || ( length != 0 && buf == nullptr )
ANI_OUT_OF_RANGE: start > array.length() || length + start > array.length()

### Array_GetRegion_Byte

`ani_status (*Array_GetRegion_Byte)(ani_env *env, ani_fixedarray_byte array, ani_size offset, ani_size length, ani_byte *native_buffer);`
This function retrieves a portion of the specified byte fixed array into a native buffer.
**PARAMETERS:**
env: A pointer to the environment structure.
array:  The fixed array to retrieve values from.
offset:  The starting offset of the region.
length: The number of elements to retrieve.
native_buffer: A buffer to store the retrieved byte values.
**RETURNS:**
Returns a status code of type ani_status indicating success or failure.
ANI_OK : retrieves a portion of the specified byte fixed array into a native buffer success
ANI_INVALID_ARGS: array == nullptr || ( length != 0 && buf == nullptr )
ANI_OUT_OF_RANGE: start > array.length() || length + start > array.length()

## 13. Reflection Support

## 14. Coroutine Support

## 15. Variable Operations

### Variable_SetValue_Long

`ani_status (*Variable_SetValue_Long)(ani_env *env, ani_variable variable, ani_long value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The long integer value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Int

`ani_status (*Variable_SetValue_Int)(ani_env *env, ani_variable variable, ani_int value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The integer value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Double

`ani_status (*Variable_SetValue_Double)(ani_env *env, ani_variable variable, ani_double value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The double value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Ref

`ani_status (*Variable_GetValue_Ref)(ani_env *env, ani_variable variable, ani_ref *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved reference value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Double

`ani_status (*Variable_GetValue_Double)(ani_env *env, ani_variable variable, ani_double *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved double value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Boolean

`ani_status (*Variable_GetValue_Boolean)(ani_env *env, ani_variable variable, ani_boolean *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved boolean value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Int

`ani_status (*Variable_GetValue_Int)(ani_env *env, ani_variable variable, ani_int *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved integer value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Long

`ani_status (*Variable_GetValue_Long)(ani_env *env, ani_variable variable, ani_long *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved long integer value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Namespace_FindVariable

`ani_status (*Namespace_FindVariable)(ani_env *env, ani_namespace ns, const char *variable_descriptor, ani_variable *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
ns: The namespace to search within.
variable_descriptor: The descriptor of the variable to find.
result: A pointer to the variable object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Boolean

`ani_status (*Variable_SetValue_Boolean)(ani_env *env, ani_variable variable, ani_boolean value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The boolean value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Char

`ani_status (*Variable_SetValue_Char)(ani_env *env, ani_variable variable, ani_char value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The character value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Byte

`ani_status (*Variable_SetValue_Byte)(ani_env *env, ani_variable variable, ani_byte value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The byte value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Short

`ani_status (*Variable_SetValue_Short)(ani_env *env, ani_variable variable, ani_short value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The short integer value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Float

`ani_status (*Variable_SetValue_Float)(ani_env *env, ani_variable variable, ani_float value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The float value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_SetValue_Ref

`ani_status (*Variable_SetValue_Ref)(ani_env *env, ani_variable variable, ani_ref value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to modify.
value: The reference value to assign to the variable.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Char

`ani_status (*Variable_GetValue_Char)(ani_env *env, ani_variable variable, ani_char *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved character value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Byte

`ani_status (*Variable_GetValue_Byte)(ani_env *env, ani_variable variable, ani_byte *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved byte value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Short

`ani_status (*Variable_GetValue_Short)(ani_env *env, ani_variable variable, ani_short *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved short integer value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Variable_GetValue_Float

`ani_status (*Variable_GetValue_Float)(ani_env *env, ani_variable variable, ani_float *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
variable: The variable to query.
result: A pointer to store the retrieved float value.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_SetStaticFieldByName_Long

`ani_status (*Class_SetStaticFieldByName_Long)(ani_env *env, ani_class cls, const char *name, ani_long value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The long value to assign.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Class_SetStaticFieldByName_Float

`ani_status (*Class_SetStaticFieldByName_Float)(ani_env *env, ani_class cls, const char *name, ani_float value);`
**PARAMETERS:**	
env: A pointer to the environment structure.
cls: The class containing the static field.
name: The name of the static field to modify.
value: The float value to assign.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
## 16. Module Support
### Module_FindNamespace
`ani_status (*Module_FindNamespace)(ani_env *env, ani_module module, const char *namespace_descriptor, ani_namespace *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
module: The module to search within.
namespace_descriptor: The descriptor of the namespace to find.
result: A pointer to the namespace object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Module_FindClass
`ani_status (*Module_FindClass)(ani_env *env, ani_module module, const char *class_descriptor, ani_class *result);`
**PARAMETERS:**	
env: A pointer to the environment structure.
module: The module to search within.
class_descriptor: The descriptor of the class to find.
result: A pointer to the class object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Module_FindEnum
`ani_status (*Module_FindEnum)(ani_env *env, ani_module module, const char *enum_descriptor, ani_enum *result);`
**PARAMETERS:** 
env: A pointer to the environment structure.
module: The module to search within.
enum_descriptor: The descriptor of the enum to find.
result: A pointer to the enum object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Module_FindFunction
`ani_status (*Module_FindFunction)(ani_env *env, ani_module module, const char *function_descriptor, ani_function *result);`
**PARAMETERS:** 
env: A pointer to the environment structure.
module: The module to search within.
name: The name of the function to retrieve.
signature: The signature of the function to retrieve.
result: A pointer to the function object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Module_FindVariable
`ani_status (*Module_FindVariable)(ani_env *env, ani_module module, const char *variable_descriptor, ani_variable *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
module: The module to search within.
variable_descriptor: The descriptor of the variable to find.
result: A pointer to the variable object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

## 17. NameSpace Support
### Namespace_FindNamespace
`ani_status (*Namespace_FindNamespace)(ani_env *env, ani_namespace ns, const char *namespace_descriptor, ani_namespace *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
ns: The parent namespace to search within.
namespace_descriptor: The descriptor of the namespace to find.
result: A pointer to the namespace object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Namespace_FindClass
`ani_status (*Namespace_FindClass)(ani_env *env, ani_namespace ns, const char *class_descriptor, ani_class *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
ns: The namespace to search within.
class_descriptor: The descriptor of the class to find.
result: A pointer to the class object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Namespace_FindEnum
`ani_status (*Namespace_FindEnum)(ani_env *env, ani_namespace ns, const char *enum_descriptor, ani_enum *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
ns: The namespace to search within.
enum_descriptor: The descriptor of the enum to find.
result: A pointer to the enum object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Namespace_FindFunction
`ani_status (*Namespace_FindFunction)(ani_env *env, ani_namespace ns, const char *function_descriptor, ani_function *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
ns: The namespace to search within.
name: The name of the function to retrieve.
signature: The signature of the function to retrieve.
result: A pointer to the function object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

## 18. Enum Operations
### Enum_GetEnumItemByName
`ani_status (*Enum_GetEnumItemByName)(ani_env *env, ani_enum enm, const char *name, ani_enum_item *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
enm: The enum to search within.
name: The name of the enum item to retrieve.
result: A pointer to store the retrieved enum item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### Enum_GetEnumItemByIndex
`ani_status (*Enum_GetEnumItemByIndex)(ani_env *env, ani_enum enm, ani_size index, ani_enum_item *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
enm: The enum to search within.
index: The index of the enum item to retrieve.
result: A pointer to store the retrieved enum item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### EnumItem_GetEnum
`ani_status (*EnumItem_GetEnum)(ani_env *env, ani_enum_item enum_item, ani_enum *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
enum_item: The enum item whose associated enum is to be retrieved.
result: A pointer to store the retrieved enum.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### EnumItem_GetValue_Int
`ani_status (*EnumItem_GetValue_Int)(ani_env *env, ani_enum_item enum_item, ani_int *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
enum_item：The enum item whose underlying value is to be retrieved.
result: A pointer to store the retrieved object.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### EnumItem_GetValue_String
`ani_status (*EnumItem_GetValue_String)(ani_env *env, ani_enum_item enum_item, ani_string *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
enum_item: The enum item whose underlying value is to be retrieved.
result: A pointer to store the retrieved string.
**RETURNS:**
return Returns a status code of type `ani_status` indicating success or failure.

### EnumItem_GetName
`ani_status (*EnumItem_GetName)(ani_env *env, ani_enum_item enum_item, ani_string *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
enum_item: The enum item whose name is to be retrieved.
result: A pointer to store the retrieved name.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### EnumItem_GetIndex
`ani_status (*EnumItem_GetIndex)(ani_env *env, ani_enum_item enum_item, ani_size *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
enum_item: The enum item whose index is to be retrieved.
result: A pointer to store the retrieved index.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

## 19. CLS Support

## 20. Tulpe Operations

### TupleValue_GetItem_Boolean
`ani_status (*TupleValue_GetItem_Boolean)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_boolean *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the boolean value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Char
`ani_status (*TupleValue_GetItem_Char)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_char *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the char value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Byte
`ani_status (*TupleValue_GetItem_Byte)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_byte *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the byte value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Short
`ani_status (*TupleValue_GetItem_Short)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_short *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the short value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Int
`ani_status (*TupleValue_GetItem_Int)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_int *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the integer value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Long
`ani_status (*TupleValue_GetItem_Long)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_long *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the long value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Float
`ani_status (*TupleValue_GetItem_Float)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_float *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the float value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Double
`ani_status (*TupleValue_GetItem_Double)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_double *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the double value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_GetItem_Ref
`ani_status (*TupleValue_GetItem_Ref)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_ref *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
result: A pointer to store the reference value of the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Boolean
`ani_status (*TupleValue_SetItem_Boolean)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_boolean value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The boolean value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Char
`ani_status (*TupleValue_SetItem_Char)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_char value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The char value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Byte
`ani_status (*TupleValue_SetItem_Byte)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_byte value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The byte value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Short
`ani_status (*TupleValue_SetItem_Short)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_short value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The short value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Int
`ani_status (*TupleValue_SetItem_Int)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_int value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The integer value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Long
`ani_status (*TupleValue_SetItem_Long)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_long value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The long value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Float
`ani_status (*TupleValue_SetItem_Float)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_float value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The float value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Double
`ani_status (*TupleValue_SetItem_Double)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_double value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The double value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### TupleValue_SetItem_Ref
`ani_status (*TupleValue_SetItem_Ref)(ani_env *env, ani_tuple_value tuple_value, ani_size index, ani_ref value);`
**PARAMETERS:**
env: A pointer to the environment structure.
tuple_value: The tuple value containing the item.
index: The index of the item.
value: The reference value to assign to the item.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

## 21. Async Operations
## 22. Scope Support

### CreateLocalScope
`ani_status (*CreateLocalScope)(ani_env *env, ani_size nr_refs);`
**PARAMETERS:**
env: A pointer to the environment structure.
nr_refs: The maximum number of references that can be created in this scope.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### DestroyLocalScope
`ani_status (*DestroyLocalScope)(ani_env *env);`
**PARAMETERS:**
env: A pointer to the environment structure.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### CreateEscapeLocalScope
`ani_status (*CreateEscapeLocalScope)(ani_env *env, ani_size nr_refs);`
**PARAMETERS:**
env: A pointer to the environment structure.
nr_refs: The maximum number of references that can be created in this scope.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### DestroyEscapeLocalScope
`ani_status (*DestroyEscapeLocalScope)(ani_env *env, ani_ref ref, ani_ref *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
ref The reference to be escaped from the current scope.
result: A pointer to the resulting reference that has escaped the scope.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
	 

## 23. Program Operations

## 24. ThreadSafe Support

## 25. Buffer Operations

## 26. TypeCast Support

## 27. Env Operations

### GetEnv
`ani_status (*GetEnv)(ani_vm *vm, uint32_t version, ani_env **result);`

## 28. JS Feature

### GetUndefined
`ani_status (*GetUndefined)(ani_env *env, ani_ref *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
result: A pointer to store the undefined reference.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

### GetNull
`ani_status (*GetNull)(ani_env *env, ani_ref *result);`
**PARAMETERS:**
env: A pointer to the environment structure.
result: A pointer to store the null reference.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.
	 

## 29. Ark Feature

## 30. VM Interface

### ani_CreateVM
`ani_EXPORT ani_status ani_CreateVM(uint32_t version, size_t argc, const ani_vm_argument *argv, ani_vm **result);`

### ani_GetCreatedVMs
`ani_EXPORT ani_status ani_GetCreatedVMs(ani_vm *vm_s_buffer, ani_size vm_s_buffer_length, ani_size *result);`

### DestroyVM
`ani_status (*DestroyVM)(ani_vm *vm);`

### GetVM
`ani_status (*GetVM)(ani_env *env, ani_vm **result);`
**PARAMETERS:**
env: A pointer to the environment structure.
result: A pointer to the VM instance to be populated.
**RETURNS:**
Returns a status code of type `ani_status` indicating success or failure.

## 31.ThreadSafe function
### AttachCurrentThread
`ani_status (*AttachCurrentThread)(ani_vm *vm, const ani_options *options, uint32_t version, ani_env **result);`

### DetachCurrentThread
`ani_status (*DetachCurrentThread)(ani_vm *vm);`