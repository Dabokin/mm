#include "custom.h"  // Сначала свой заголовок
#include "imgui_internal.h"
#include <string>

namespace CustomWidgets {

// Кастомный чекбокс с анимацией
bool checkboxCs(const char* label, bool* v, const ImVec4& gradient_color) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float square_size = ImGui::GetFrameHeight();
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_size + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f),
        pos.y + ImMax(square_size, label_size.y)));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
    }

    // Анимация галочки
    static ImGuiStorage* storage = window->DC.StateStorage;
    float& anim = *storage->GetFloatRef(id, 0.0f);

    if (ImGui::IsItemActivated())
        anim = 0.0f;

    const float target = *v ? 1.0f : 0.0f;
    anim = ImLerp(anim, target, g.IO.DeltaTime * 12.0f);

    // Анимация обводки
    float& hover_anim = *storage->GetFloatRef(id + 1, 0.0f);
    hover_anim = ImClamp(hover_anim + (hovered ? g.IO.DeltaTime * 4.0f : -g.IO.DeltaTime * 4.0f), 0.0f, 1.0f);

    // Рисуем фон
    const ImRect check_bb(pos, ImVec2(pos.x + square_size, pos.y + square_size));
    const float rounding = style.FrameRounding;
    window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), rounding);

    // Градиентная обводка
    if (hover_anim > 0.0f) {
        const float thickness = ImLerp(1.0f, 2.5f, hover_anim);
        const ImU32 color_start = ImGui::GetColorU32(gradient_color);
        const ImU32 color_end = ImAlphaBlendColors(color_start, ImGui::GetColorU32(ImGuiCol_Border));

        // Рисуем градиентный контур
        ImVec2 points[4] = {
            check_bb.Min,
            ImVec2(check_bb.Max.x, check_bb.Min.y),
            check_bb.Max,
            ImVec2(check_bb.Min.x, check_bb.Max.y)
        };

        window->DrawList->AddPolyline(points, 4, color_start, true, thickness);
        window->DrawList->AddRect(ImVec2(check_bb.Min.x + 1, check_bb.Min.y + 1),
            ImVec2(check_bb.Max.x - 1, check_bb.Max.y - 1),
            color_end, rounding, 0, thickness * 0.5f);
    }

    // Анимированная галочка
    if (anim > 0.01f) {
        const float pad = ImMax(1.0f, square_size / 6.0f);
        const float r = square_size - pad;

        // Плавное проявление
        const float alpha = ImClamp(anim * 2.0f, 0.0f, 1.0f);
        const float scale = ImClamp(anim * 1.2f - 0.1f, 0.0f, 1.0f);

        const ImVec2 center = check_bb.GetCenter();
        ImVec2 points[3] = {
            ImVec2(center.x - r * 0.3f, center.y - r * 0.1f),
            ImVec2(center.x - r * 0.1f, center.y + r * 0.3f),
            ImVec2(center.x + r * 0.4f, center.y - r * 0.3f)
        };

        // Масштабируем и смещаем
        for (int i = 0; i < 3; i++) {
            points[i].x = center.x + (points[i].x - center.x) * scale;
            points[i].y = center.y + (points[i].y - center.y) * scale;
        }

        window->DrawList->AddPolyline(points, 3,
            ImAlphaBlendColors(ImGui::GetColorU32(ImGuiCol_CheckMark), IM_COL32(255, 255, 255, static_cast<int>(255 * alpha))),
            false,
            ImMax(2.0f, square_size * 0.15f)
        );
    }

    // Текст
    if (label_size.x > 0.0f) {
        ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y), label);
    }

    return pressed;
}

bool ToggleSwitch(const char* label, bool* v) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(label, ImVec2(width, height));
    if (ImGui::IsItemClicked()) *v = !*v;

    float t = *v ? 1.0f : 0.0f;
    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(label)) {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? t_anim : 1.0f - t_anim;
    }

    ImU32 col_bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 col_circle = ImGui::GetColorU32(ImGuiCol_CheckMark);
    if (ImGui::IsItemHovered()) {
        col_bg = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
    }

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, col_circle);

    return *v;
}



bool ComboCs(const char* label, int* current_item, const char* const items[], int items_count, float anim_speed) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float arrow_size = ImGui::GetFrameHeight() * 0.3f;
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const float width = ImGui::CalcItemWidth();
    const float height = ImGui::GetFrameHeight();

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect frame_bb(pos, ImVec2(pos.x + width, pos.y + height));
    const ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + height));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(frame_bb, id, &hovered, &held);
    bool popup_open = ImGui::IsPopupOpen(id, ImGuiPopupFlags_None);

    // Анимации
    static ImGuiStorage* storage = window->DC.StateStorage;
    float& hover_anim = *storage->GetFloatRef(id + 1, 0.0f);
    float& open_anim = *storage->GetFloatRef(id + 2, 0.0f);

    hover_anim = ImClamp(hover_anim + (hovered ? g.IO.DeltaTime * 6.0f : -g.IO.DeltaTime * 6.0f), 0.0f, 1.0f);

    if (pressed && !popup_open) {
        ImGui::OpenPopupEx(id, ImGuiPopupFlags_None);
        open_anim = 0.0f;
    }

    if (popup_open) {
        open_anim = ImMin(open_anim + g.IO.DeltaTime * anim_speed, 1.0f);
    }
    else {
        open_anim = ImMax(open_anim - g.IO.DeltaTime * anim_speed, 0.0f);
    }

    // Рисуем фон
    const float rounding = 4.0f;
    const ImU32 bg_color = ImGui::GetColorU32(ImVec4(0.11f, 0.11f, 0.11f, 0.95f));
    window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, bg_color, rounding);

    // Тонкая серая обводка
    const float border_thickness = 0.8f;
    const ImU32 border_color = ImGui::GetColorU32(ImVec4(0.35f, 0.35f, 0.35f, 0.5f));
    window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, border_color, rounding, 0, border_thickness);

    // Эффект при наведении
    if (hover_anim > 0.0f) {
        const ImU32 hover_color = ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, hover_anim * 0.3f));
        window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, hover_color, rounding);
    }

    // Текущее значение
    const char* preview_value = *current_item >= 0 && *current_item < items_count ? items[*current_item] : "";
    ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x + style.FramePadding.x + 2.0f, frame_bb.Min.y + style.FramePadding.y),
        frame_bb.Max, preview_value, NULL, NULL);

    // Стрелка
    const float arrow_x = frame_bb.Max.x - arrow_size - style.FramePadding.x - 4.0f;
    const float arrow_y = frame_bb.Min.y + (frame_bb.Max.y - frame_bb.Min.y) * 0.5f;

    const float arrow_rotation = ImLerp(0.0f, IM_PI * 0.5f, open_anim);
    ImVec2 arrow_points[3] = {
        ImVec2(-arrow_size * 0.5f, arrow_size * 0.3f),
        ImVec2(arrow_size * 0.5f, arrow_size * 0.3f),
        ImVec2(0.0f, -arrow_size * 0.3f)
    };

    for (int i = 0; i < 3; i++) {
        float x = arrow_points[i].x;
        float y = arrow_points[i].y;
        arrow_points[i].x = arrow_x + x * cosf(arrow_rotation) - y * sinf(arrow_rotation);
        arrow_points[i].y = arrow_y + x * sinf(arrow_rotation) + y * cosf(arrow_rotation);
    }

    window->DrawList->AddTriangleFilled(arrow_points[0], arrow_points[1], arrow_points[2], ImGui::GetColorU32(ImVec4(0.8f, 0.8f, 0.8f, 0.9f)));

    // Текст метки
    if (label_size.x > 0.0f) {
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);
    }

    // Выпадающий список
    if (open_anim > 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, open_anim);

        if (ImGui::BeginPopupEx(id, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration)) {
            const float item_height = ImGui::GetTextLineHeightWithSpacing();
            const float max_height = item_height * ImMin(items_count, 8) + style.FramePadding.y * 2.0f;

            // Устанавливаем позицию под комбо-боксом
            ImGui::SetWindowPos(ImVec2(frame_bb.Min.x, frame_bb.Max.y - 1.0f));
            ImGui::SetWindowSize(ImVec2(frame_bb.GetWidth(), 0.0f)); // Высота будет анимирована

            // Темный фон списка
            const ImU32 list_bg_color = ImGui::GetColorU32(ImVec4(0.09f, 0.09f, 0.09f, 0.99f));
            const ImU32 list_border_color = ImGui::GetColorU32(ImVec4(0.35f, 0.35f, 0.35f, 0.5f));

            // Анимируем высоту
            const float anim_height = ImLerp(0.0f, max_height, open_anim);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, border_thickness);

            window->DrawList->AddRectFilled(
                ImVec2(frame_bb.Min.x, frame_bb.Max.y - 1.0f),
                ImVec2(frame_bb.Max.x, frame_bb.Max.y + anim_height),
                list_bg_color, rounding, ImDrawFlags_RoundCornersBottom);

            window->DrawList->AddRect(
                ImVec2(frame_bb.Min.x, frame_bb.Max.y - 1.0f),
                ImVec2(frame_bb.Max.x, frame_bb.Max.y + anim_height),
                list_border_color, rounding, ImDrawFlags_RoundCornersBottom, border_thickness);

            ImGui::BeginChild("##combo_scroll", ImVec2(frame_bb.GetWidth(), anim_height), false,
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 0.7f));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

            for (int i = 0; i < items_count; i++) {
                const bool is_selected = (*current_item == i);
                if (ImGui::Selectable(items[i], is_selected)) {
                    *current_item = i;
                    ImGui::CloseCurrentPopup();
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::EndPopup();
        }

        ImGui::PopStyleVar();
    }

    return pressed;
}




}
