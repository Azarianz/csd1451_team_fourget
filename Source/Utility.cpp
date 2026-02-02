#include "Utility.h"

namespace Utility {
	bool IsCircleClicked(float circle_center_x, float circle_center_y, float radius, float click_x, float click_y)
	{
		float dx = click_x - circle_center_x;
		float dy = click_y - circle_center_y;

		if (dx * dx + dy * dy <= radius * radius) {
			return true;
		}
		else {
			return false;
		}
	}

	void GetWorldMousePos(float& worldX, float& worldY) {
		int mouseX, mouseY;
		AEInputGetCursorPosition(&mouseX, &mouseY);
		worldX = (float)mouseX - (AEGfxGetWindowWidth() / 2.f);
		worldY = (AEGfxGetWindowHeight() / 2.f) - (float)mouseY;
	}
}
