#pragma once
#include "AEEngine.h"

namespace Utility {
	bool IsCircleClicked(float circle_center_x, float circle_center_y, float diameter, float click_x, float click_y);
	void GetWorldMousePos(float& worldX, float& worldY);
}
