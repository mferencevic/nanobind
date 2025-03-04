#include <nanobind/stl/tuple.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/list.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/map.h>

NB_MAKE_OPAQUE(NB_TYPE(std::vector<float, std::allocator<float>>))

namespace nb = nanobind;

static int default_constructed = 0, value_constructed = 0, copy_constructed = 0,
           move_constructed = 0, copy_assigned = 0, move_assigned = 0,
           destructed = 0;

struct Movable {
    int value = 5;

    Movable() { default_constructed++; }
    Movable(int value) : value(value) { value_constructed++; }
    Movable(const Movable &s) : value(s.value) { copy_constructed++; }
    Movable(Movable &&s) noexcept : value(s.value) { s.value = 0; move_constructed++; }
    Movable &operator=(const Movable &s) { value = s.value; copy_assigned++; return *this; }
    Movable &operator=(Movable &&s) noexcept { std::swap(value, s.value); move_assigned++; return *this; }
    ~Movable() { destructed++; }
};

struct Copyable {
    int value = 5;

    Copyable() { default_constructed++; }
    Copyable(int value) : value(value) { value_constructed++; }
    Copyable(const Copyable &s) : value(s.value) { copy_constructed++; }
    Copyable &operator=(const Copyable &s) { value = s.value; copy_assigned++; return *this; }
    ~Copyable() { destructed++; }
};

void fail() { throw std::exception(); }

NB_MODULE(test_stl_ext, m) {
    m.def("stats", []{
        nb::dict d;
        d["default_constructed"] = default_constructed;
        d["value_constructed"] = value_constructed;
        d["copy_constructed"] = copy_constructed;
        d["move_constructed"] = move_constructed;
        d["copy_assigned"] = copy_assigned;
        d["move_assigned"] = move_assigned;
        d["destructed"] = destructed;
        return d;
    });

    m.def("reset", []() {
        default_constructed = 0;
        value_constructed = 0;
        copy_constructed = 0;
        move_constructed = 0;
        copy_assigned = 0;
        move_assigned = 0;
        destructed = 0;
    });

    nb::class_<Movable>(m, "Movable")
        .def(nb::init<>())
        .def(nb::init<int>())
        .def_readwrite("value", &Movable::value);

    nb::class_<Copyable>(m, "Copyable")
        .def(nb::init<>())
        .def(nb::init<int>())
        .def_readwrite("value", &Copyable::value);

    // ----- test01-test12 ------ */

    m.def("return_movable", []() { return Movable(); });
    m.def("return_movable_ptr", []() { return new Movable(); });
    m.def("movable_in_value", [](Movable m) { if (m.value != 5) fail(); });
    m.def("movable_in_lvalue_ref", [](Movable &m) { if (m.value != 5) fail(); });
    m.def("movable_in_rvalue_ref", [](Movable &&m) { Movable x(std::move(m)); if (x.value != 5) fail(); });
    m.def("movable_in_ptr", [](Movable *m) { if (m->value != 5) fail(); });
    m.def("return_copyable", []() { return Copyable(); });
    m.def("return_copyable_ptr", []() { return new Copyable(); });
    m.def("copyable_in_value", [](Copyable m) { if (m.value != 5) fail(); });
    m.def("copyable_in_lvalue_ref", [](Copyable &m) { if (m.value != 5) fail(); });
    m.def("copyable_in_rvalue_ref", [](Copyable &&m) { Copyable x(m); if (x.value != 5) fail(); });
    m.def("copyable_in_ptr", [](Copyable *m) { if (m->value != 5) fail(); });

    // ----- test13-test20 ------ */

    m.def("tuple_return_movable", []() { return std::make_tuple(Movable()); });
    m.def("tuple_return_movable_ptr", []() { return std::make_tuple(new Movable()); });
    m.def("tuple_movable_in_value", [](std::tuple<Movable> m) { if (std::get<0>(m).value != 5) fail(); });
    m.def("tuple_movable_in_lvalue_ref", [](std::tuple<Movable&> m) { if (std::get<0>(m).value != 5) fail(); });
    m.def("tuple_movable_in_lvalue_ref_2", [](const std::tuple<Movable> &m) { if (std::get<0>(m).value != 5) fail(); });
    m.def("tuple_movable_in_rvalue_ref", [](std::tuple<Movable&&> m) { Movable x(std::move(std::get<0>(m))); if (x.value != 5) fail(); });
    m.def("tuple_movable_in_rvalue_ref_2", [](std::tuple<Movable> &&m) { Movable x(std::move(std::get<0>(m))); if (x.value != 5) fail(); });
    m.def("tuple_movable_in_ptr", [](std::tuple<Movable*> m) { if (std::get<0>(m)->value != 5) fail(); });

    // ----- test21 ------ */

    m.def("empty_tuple", [](std::tuple<>) { return std::tuple<>(); });
    m.def("swap_tuple", [](const std::tuple<int, float> &v) {
        return std::tuple<float, int>(std::get<1>(v), std::get<0>(v));
    });
    m.def("swap_pair", [](const std::pair<int, float> &v) {
        return std::pair<float, int>(std::get<1>(v), std::get<0>(v));
    });

    // ----- test22 ------ */
    m.def("vec_return_movable", [](){
        std::vector<Movable> x;
        x.reserve(10);
        for (int i = 0; i< 10; ++i)
            x.emplace_back(i);
        return x;
    });

    m.def("vec_return_copyable", [](){
        std::vector<Copyable> x;
        x.reserve(10);
        for (int i = 0; i < 10; ++i) {
            Copyable c(i);
            x.push_back(c);
        }
        return x;
    });

    m.def("vec_moveable_in_value", [](std::vector<Movable> x) {
        if (x.size() != 10)
            fail();
        for (int i = 0; i< 10; ++i)
            if (x[i].value != i)
                fail();
    });


    m.def("vec_copyable_in_value", [](std::vector<Copyable> x) {
        if (x.size() != 10)
            fail();
        for (int i = 0; i< 10; ++i)
            if (x[i].value != i)
                fail();
    });


    m.def("vec_moveable_in_lvalue_ref", [](std::vector<Movable> &x) {
        if (x.size() != 10)
            fail();
        for (int i = 0; i< 10; ++i)
            if (x[i].value != i)
                fail();
    });


    m.def("vec_moveable_in_rvalue_ref", [](std::vector<Movable> &&x) {
        if (x.size() != 10)
            fail();
        for (int i = 0; i< 10; ++i)
            if (x[i].value != i)
                fail();
    });

    m.def("vec_moveable_in_ptr_2", [](std::vector<Movable *> x) {
        if (x.size() != 10)
            fail();
        for (int i = 0; i< 10; ++i)
            if (x[i]->value != i)
                fail();
    });

    // ----- test29 ------ */
    using fvec = std::vector<float, std::allocator<float>>;
    nb::class_<fvec>(m, "float_vec")
        .def(nb::init<>())
        .def("push_back", [](fvec *fv, float f) { fv->push_back(f); })
        .def("size", [](const fvec &fv) { return fv.size(); });

    // ----- test30 ------ */

    m.def("return_empty_function", []() -> std::function<int(int)> {
        return {};
    });
    m.def("return_function", []() -> std::function<int(int)> {
        int k = 5;
        return [k](int l) { return k + l; };
    });

    m.def("call_function", [](std::function<int(int)> &f, int x) {
        return f(x);
    });

    m.def("identity_list", [](std::list<int> &x) { return x; });

    // ----- test33 ------ */
    m.def("identity_string", [](std::string& x) { return x; });
    m.def("identity_string_view", [](std::string_view& x) { return x; });

    // ----- test34-test40 ------ */
    m.def("optional_copyable", [](std::optional<Copyable> &) {}, nb::arg("x").none());
    m.def("optional_copyable_ptr", [](std::optional<Copyable *> &) {}, nb::arg("x").none());
    m.def("optional_none", [](std::optional<Copyable> &x) { if(x) fail(); }, nb::arg("x").none());
    m.def("optional_ret_opt_movable", []() { return std::optional<Movable>(Movable()); });
    m.def("optional_ret_opt_movable_ptr", []() { return new std::optional<Movable *>(new Movable()); });
    m.def("optional_ret_opt_none", []() { return std::optional<Movable>(); });
    m.def("optional_unbound_type", [](std::optional<int> &x) { return x; }, nb::arg("x").none() = nb::none());

    // ----- test41-test47 ------ */
    m.def("variant_copyable", [](std::variant<Copyable, int> &) {});
    m.def("variant_copyable_none", [](std::variant<std::monostate, Copyable, int> &) {}, nb::arg("x").none());
    m.def("variant_copyable_ptr", [](std::variant<Copyable *, int> &) {});
    m.def("variant_copyable_ptr_none", [](std::variant<Copyable *, int> &) {}, nb::arg("x").none());
    m.def("variant_ret_var_copyable", []() { return std::variant<Copyable, int>(); });
    m.def("variant_ret_var_none", []() { return std::variant<std::monostate, Copyable, int>(); });
    m.def("variant_unbound_type", [](std::variant<std::monostate, nb::list, nb::tuple, int> &x) { return x; },
          nb::arg("x").none() = nb::none());

    // ----- test48-test54 ------ */
    m.def("map_return_movable_value", [](){
        std::map<std::string, Movable> x;
        for (int i = 0; i < 10; ++i)
            x.emplace(std::string(1, 'a' + i), i);
        return x;
    });
    m.def("map_return_copyable_value", [](){
        std::map<std::string, Copyable> x;
        for (int i = 0; i < 10; ++i) {
            Copyable c(i);
            x.insert({std::string(1, 'a' + i), c});
        }
        return x;
    });
    m.def("map_movable_in_value", [](std::map<std::string, Movable> x) {
        if (x.size() != 10) fail();
        for (int i = 0; i < 10; ++i) {
            std::string key(1, 'a' + i);
            if (x.find(key) == x.end()) fail();
            if (x[key].value != i) fail();
        }
    }, nb::arg("x"));
    m.def("map_copyable_in_value", [](std::map<std::string, Copyable> x) {
        if (x.size() != 10) fail();
        for (int i = 0; i < 10; ++i) {
            std::string key(1, 'a' + i);
            if (x.find(key) == x.end()) fail();
            if (x[key].value != i) fail();
        }
    }, nb::arg("x"));
    m.def("map_movable_in_lvalue_ref", [](std::map<std::string, Movable> &x) {
        if (x.size() != 10) fail();
        for (int i = 0; i < 10; ++i) {
            std::string key(1, 'a' + i);
            if (x.find(key) == x.end()) fail();
            if (x[key].value != i) fail();
        }
    }, nb::arg("x"));
    m.def("map_movable_in_rvalue_ref", [](std::map<std::string, Movable> &&x) {
        if (x.size() != 10) fail();
        for (int i = 0; i < 10; ++i) {
            std::string key(1, 'a' + i);
            if (x.find(key) == x.end()) fail();
            if (x[key].value != i) fail();
        }
    }, nb::arg("x"));
    m.def("map_movable_in_ptr", [](std::map<std::string, Movable *> x) {
        if (x.size() != 10) fail();
        for (int i = 0; i < 10; ++i) {
            std::string key(1, 'a' + i);
            if (x.find(key) == x.end()) fail();
            if (x[key]->value != i) fail();
        }
    }, nb::arg("x"));
}
