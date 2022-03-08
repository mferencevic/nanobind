/*
    nanobind/nb_lib.h: Interface to libnanobind.so

    Copyright (c) 2022 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)

/**
 * Helper class to clean temporaries created by function dispatch.
 * The first element serves a special role: it stores the 'self'
 * object of method calls (for rv_policy::reference_internal).
 */
struct NB_CORE cleanup_list {
public:
    static constexpr uint32_t Small = 6;

    cleanup_list(PyObject *self) {
        m_size = 1;
        m_capacity = Small;
        m_data = m_local;
        m_local[0] = self;
    }

    ~cleanup_list() = default;

    /// Append a single PyObject to the cleanup stack
    NB_INLINE void append(PyObject *value) noexcept {
        if (m_size >= m_capacity)
            expand();
        m_data[m_size++] = value;
    }

    NB_INLINE PyObject *self() const {
        return m_local[0];
    }

    /// Decrease the reference count of all appended objects
    void release() noexcept;

protected:
    /// Out of memory, expand..
    void expand() noexcept;

protected:
    uint32_t m_size;
    uint32_t m_capacity;
    PyObject **m_data;
    PyObject *m_local[Small];
};


// ========================================================================

/// Raise a std::runtime_error with the given message
#if defined(__GNUC__)
    __attribute__((noreturn, __format__ (__printf__, 1, 2)))
#else
    [[noreturn]]
#endif
NB_CORE void raise(const char *fmt, ...);

/// Abort the process with a fatal error
#if defined(__GNUC__)
    __attribute__((noreturn, __format__ (__printf__, 1, 2)))
#else
    [[noreturn]]
#endif
NB_CORE void fail(const char *fmt, ...) noexcept;

/// Raise nanobind::python_error after an error condition was found
NB_CORE void raise_python_error();

/// Raise nanobind::next_overload
NB_CORE void raise_next_overload();

// ========================================================================

/// Convert a Python object into a Python unicode string
NB_CORE PyObject *str_from_obj(PyObject *o);

/// Convert an UTF8 null-terminated C string into a Python unicode string
NB_CORE PyObject *str_from_cstr(const char *c);

/// Convert an UTF8 C string + size into a Python unicode string
NB_CORE PyObject *str_from_cstr_and_size(const char *c, size_t n);

// ========================================================================

/// Get an object attribute or raise an exception
NB_CORE PyObject *getattr(PyObject *obj, const char *key);
NB_CORE PyObject *getattr(PyObject *obj, PyObject *key);

/// Get an object attribute or return a default value (never raises)
NB_CORE PyObject *getattr(PyObject *obj, const char *key, PyObject *def) noexcept;
NB_CORE PyObject *getattr(PyObject *obj, PyObject *key, PyObject *def) noexcept;

/// Get an object attribute or raise an exception. Skip if 'out' is non-null
NB_CORE void getattr_maybe(PyObject *obj, const char *key, PyObject **out);
NB_CORE void getattr_maybe(PyObject *obj, PyObject *key, PyObject **out);

/// Set an object attribute / item
NB_CORE void setattr(PyObject *obj, const char *key, PyObject *value);
NB_CORE void setattr(PyObject *obj, PyObject *key, PyObject *value);

// ========================================================================

/// Index into an object or raise an exception. Skip if 'out' is non-null
NB_CORE void getitem_maybe(PyObject *obj, Py_ssize_t, PyObject **out);
NB_CORE void getitem_maybe(PyObject *obj, const char *key, PyObject **out);
NB_CORE void getitem_maybe(PyObject *obj, PyObject *key, PyObject **out);

/// Set an item or raise an exception
NB_CORE void setitem(PyObject *obj, Py_ssize_t, PyObject *value);
NB_CORE void setitem(PyObject *obj, const char *key, PyObject *value);
NB_CORE void setitem(PyObject *obj, PyObject *key, PyObject *value);

// ========================================================================

/// Determine the length of a Python object
NB_CORE size_t obj_len(PyObject *o);

/// Obtain a string representation of a Python object
NB_CORE PyObject* obj_repr(PyObject *o);

/// Perform a comparison between Python objects and handle errors
NB_CORE bool obj_comp(PyObject *a, PyObject *b, int value);

/// Perform an unary operation on a Python object with error handling
NB_CORE PyObject *obj_op_1(PyObject *a, PyObject* (*op)(PyObject*));

/// Perform an unary operation on a Python object with error handling
NB_CORE PyObject *obj_op_2(PyObject *a, PyObject *b,
                           PyObject *(*op)(PyObject *, PyObject *));

// Perform a vector function call
NB_CORE PyObject *obj_vectorcall(PyObject *base, PyObject *const *args,
                                 size_t nargsf, PyObject *kwnames,
                                 bool method_call);

// ========================================================================

// Conversion validity check done by nb::make_tuple
NB_CORE void tuple_check(PyObject *tuple, size_t nargs);

// ========================================================================

// Append a single argument to a function call
NB_CORE void call_append_arg(PyObject *args, size_t &nargs, PyObject *value);

// Append a variable-length sequence of arguments to a function call
NB_CORE void call_append_args(PyObject *args, size_t &nargs, PyObject *value);

// Append a single keyword argument to a function call
NB_CORE void call_append_kwarg(PyObject *kwargs, const char *name, PyObject *value);

// Append a variable-length dictionary of keyword arguments to a function call
NB_CORE void call_append_kwargs(PyObject *kwargs, PyObject *value);

// ========================================================================

// If the given sequence has the size 'size', return a pointer to its contents.
// May produce a temporary.
NB_CORE PyObject **seq_get_with_size(PyObject *seq, size_t size,
                                     PyObject **temp) noexcept;

// Like the above, but return the size instead of checking it.
NB_CORE PyObject **seq_get(PyObject *seq, size_t *size,
                           PyObject **temp) noexcept;

// ========================================================================

/// Create a new capsule object
NB_CORE PyObject *capsule_new(const void *ptr, void (*free)(void *)) noexcept;

// ========================================================================

/// Create a Python function object for the given function record
NB_CORE PyObject *nb_func_new(const void *data) noexcept;

// ========================================================================

/// Create a Python type object for the given type record
struct type_data;
NB_CORE PyObject *nb_type_new(const type_data *c) noexcept;

/// Extract a pointer to a C++ type underlying a Python object, if possible
NB_CORE bool nb_type_get(const std::type_info *t, PyObject *o, uint8_t flags,
                         cleanup_list *cleanup, void **out) noexcept;

/// Cast a C++ type instance into a Python object
NB_CORE PyObject *nb_type_put(const std::type_info *cpp_type, void *value,
                              rv_policy rvp, cleanup_list *cleanup,
                              bool *is_new) noexcept;

// Special version of 'nb_type_put' for unique pointers and ownership transfer
NB_CORE PyObject *nb_type_put_unique(const std::type_info *cpp_type, void *value,
                             cleanup_list *cleanup, bool cpp_delete) noexcept;

/// Try to reliquish ownership from Python object to a unique_ptr
NB_CORE void nb_type_relinquish_ownership(PyObject *o, bool cpp_delete);

/// Get a pointer to a user-defined 'extra' value associated with the nb_type t.
NB_CORE void *nb_type_extra(PyObject *t) noexcept;

/// Get a pointer to the instance data of a nanobind instance (nb_inst)
NB_CORE void *nb_inst_data(PyObject *o) noexcept;

/// Check if a Python type object wraps an instance of a specific C++ type
NB_CORE bool nb_type_isinstance(PyObject *obj, const std::type_info *t) noexcept;

/// Search for the Python type object associated with a C++ type
NB_CORE PyObject *nb_type_lookup(const std::type_info *t) noexcept;

// ========================================================================

// Create and install a Python property object
NB_CORE void property_install(PyObject *scope, const char *name, bool is_static,
                              PyObject *getter, PyObject *setter) noexcept;

// ========================================================================

NB_CORE PyObject *get_override(void *ptr, const std::type_info *type,
                               const char *name, bool pure);

// ========================================================================

// Ensure that 'patient' cannot be GCed while 'nurse' is alive
NB_CORE void keep_alive(PyObject *nurse, PyObject *patient) noexcept;

// Keep 'payload' alive until 'nurse' is GCed
NB_CORE void keep_alive(PyObject *nurse, void *payload,
                        void (*deleter)(void *) noexcept) noexcept;

// ========================================================================

/// Indicate to nanobind that an implicit constructor can convert 'src' -> 'dst'
NB_CORE void implicitly_convertible(const std::type_info *src,
                                    const std::type_info *dst) noexcept;

/// Register a callback to check if implicit conversion to 'dst' is possible
NB_CORE void implicitly_convertible(bool (*predicate)(PyObject *,
                                                      cleanup_list *),
                                    const std::type_info *dst) noexcept;

// ========================================================================

/// Add an entry to an enumeration
NB_CORE void nb_enum_put(PyObject *type, const char *name, const void *value,
                         const char *doc) noexcept;

// ========================================================================

/// Try to import a Python extension module, raises an exception upon failure
NB_CORE PyObject *module_import(const char *name);

/// Create a new extension module with the given name
NB_CORE PyObject *module_new(const char *name, PyModuleDef *def) noexcept;

/// Create a submodule of an existing module
NB_CORE PyObject *module_new_submodule(PyObject *base, const char *name,
                                       const char *doc) noexcept;

// ========================================================================

/// Print to stdout using Python
NB_CORE void print(PyObject *file, PyObject *str, PyObject *end);

NAMESPACE_END(detail)
NAMESPACE_END(NB_NAMESPACE)
