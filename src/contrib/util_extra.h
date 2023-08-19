#pragma once
#include <luadef.hpp>
#include <opencv2/core/utility.hpp>

// CV_EXPORTS_W : include this file in lua_generated_include

namespace cvextra {
	void redirectError(sol::safe_function errCallback, sol::object userdata = sol::lua_nil);
}
