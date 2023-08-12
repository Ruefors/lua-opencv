// #include <my_object/my_object.hpp>
#include <register_all.hpp>

namespace {
	struct kwargs_iterator_state {
		using it_t = NamedParameters::Base::iterator;
		it_t it;
		it_t last;

		kwargs_iterator_state(NamedParameters& kwargs)
			: it(kwargs.begin()), last(kwargs.end()) {
		}
	};

	// this gets called
	// to start the first iteration, and every
	// iteration there after
	auto kwargs_next(sol::user<kwargs_iterator_state&> user_it_state, sol::this_state ts) {
		// the state you passed in my_pairs is argument 1
		// the key value is argument 2, but we do not
		// care about the key value here
		kwargs_iterator_state& it_state = user_it_state;
		auto& it = it_state.it;
		if (it == it_state.last) {
			// return nil to signify that
			// there's nothing more to work with.
			return std::make_tuple(sol::object(sol::lua_nil),
				sol::object(sol::lua_nil));
		}

		// get the next pair
		auto itderef = *it;

		// 2 values are returned (pushed onto the stack):
		// the key and the value
		// the state is left alone
		auto r = std::make_tuple(
			sol::object(ts, sol::in_place, it->first),
			sol::object(ts, sol::in_place, it->second));

		// the iterator must be moved forward one before we return
		std::advance(it, 1);

		return r;
	}

	// pairs expects 3 returns:
	// the "next" function on how to advance,
	// the "table" itself or some state,
	// and an initial key value (can be nil)
	auto kwargs_pairs(NamedParameters& self) {
		// prepare our state
		kwargs_iterator_state it_state(self);

		// sol::user is a space/time optimization over regular
		// usertypes, it's incompatible with regular usertypes and
		// stores the type T directly in lua without any pretty
		// setup saves space allocation and a single dereference
		return std::make_tuple(&kwargs_next,
			sol::user<kwargs_iterator_state>(std::move(it_state)),
			sol::lua_nil);
	}

	auto kwargs_index(NamedParameters& self, const std::string& key) {
		return self.at(key);
	}

	void kwargs_new_index(NamedParameters& self, const std::string& key, sol::object val) {
		self.insert_or_assign(key, val);
	}

	void register_kwargs(sol::state_view& lua, sol::table& module) {
		auto kwargs = module.new_usertype<NamedParameters>("kwargs",
			// kwargs.new(...) -- dot syntax, no "self" value
			sol::meta_function::construct, sol::constructors<NamedParameters(), NamedParameters(NamedParameters::Table)>(),

			// kwargs(...) syntax, only
			sol::call_constructor, sol::constructors<NamedParameters(), NamedParameters(NamedParameters::Table)>()
		);

		kwargs[sol::meta_function::pairs] = &kwargs_pairs;

		kwargs[sol::meta_function::index] = &kwargs_index;
		kwargs.set_function("get", &kwargs_index);

		kwargs[sol::meta_function::new_index] = &kwargs_new_index;
		kwargs.set_function("set", &kwargs_new_index);

		kwargs.set_function("has", [](NamedParameters& self, const std::string& key) {
			return self.count(key) != 0;
			});

		kwargs.set_function("size", [](NamedParameters& self) {
			return self.size();
			});

		kwargs.set_function("is_instance", [](sol::object obj) {
			return obj.is<NamedParameters>();
			});
	}
}

namespace LUA_MODULE_NAME {
	std::vector<std::function<void(sol::state_view&, sol::table&)>>& lua_module_get_functions() {
		static std::vector<std::function<void(sol::state_view&, sol::table&)>> functions;
		return functions;
	}

	sol::table LUA_MODULE_OPEN(sol::this_state L) {
		sol::state_view lua(L);
		sol::table module = lua.create_table();

		// regitster_my_object(lua, module);

		register_kwargs(lua, module);
		register_all(lua, module);
		register_extension(lua, module);

		return module;
	}

	int deny_new_index(lua_State* L) {
		return luaL_error(L, "Hacking is good. Rebuild it yourself without this protection!");
	}
}

int LUA_MODULE_LUAOPEN(lua_State* L) {
	// pass the lua_State,
	// the index to start grabbing arguments from,
	// and the function itself
	// optionally, you can pass extra arguments to the function
	// if that's necessary, but that's advanced usage and is
	// generally reserved for internals only
	return sol::stack::call_lua(
		L, 1, LUA_MODULE_NAME::LUA_MODULE_OPEN);
}
