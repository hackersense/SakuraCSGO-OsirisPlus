#pragma once

#include <array>
#include <imgui/imgui.h>

#include "../SDK/EngineTrace.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Vector.h"

#include "../Interfaces.h"
#include "../Memory.h"

namespace GrenadePrediction {
	void run(UserCmd* cmd) noexcept;
	void render() noexcept;
};