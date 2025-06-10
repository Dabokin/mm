#pragma once
#include "imgui.h"


namespace CustomWidgets {

bool checkboxCs(const char* label, bool* v, const ImVec4& gradient_color);
bool ToggleSwitch(const char* label, bool* v);
bool ComboCs(const char* label, int* current_item, const char* const items[], int items_count, float anim_speed = 8.0f);



}
