#pragma once

namespace hpack {

#include <fstream>
#include <unordered_map> 

#ifndef HPACK_ASSERT
#include <cassert>
#define HPACK_ASSERT(_EXPR) assert(_EXPR);
#endif
#define HPACK_UNUSED(x) ((void)(x));

	// Forward declarations
	struct Context;
	struct Rectangle;



} //namespace hpack


