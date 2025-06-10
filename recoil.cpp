//#include "imgui.h"
//#include <random>
//#include <Windows.h>
//#include <chrono>
//#include <vector>
//#include <algorithm>
//#include "Datarecoil.hpp"
//
//// Глобальные переменные
//std::random_device rd;
//std::mt19937 gen(rd());
//static bool recoil_enabled = false;
//bool f2_pressed = false;
//static bool is_recoil_active = false;
//
//// Настройки
//static int screen_width = 1920;
//static int screen_height = 1080;
//static float resolution_factor = 1.0f;
//static int fov = 90;
//float sensitivity = 0.30f;
//int randomization_level = 100;
//int scope = 0;
//int barrel = 0;
//float rcsX = 100.0f;
//float rcsY = 100.0f;
//
//// Структуры данных
//struct WeaponRCSSettings {
//    float rcsX;
//    float rcsY;
//    const char* name;
//};
//
//std::vector<WeaponRCSSettings> weaponSettings = {
//    {0, 0, "None"},
//    {100, 100, "AK47"},
//    {100, 100, "LR300"},
//    {100, 100, "MP5"},
//    {100, 100, "SMG"},
//    {100, 100, "Thompson"},
//    {100, 100, "M249"},
//    {100, 100, "M39"},
//    {100, 100, "Semi"}
//};
//
//// Инициализация системы
//void InitRecoilSystem() {
//    RECT desktop;
//    GetWindowRect(GetDesktopWindow(), &desktop);
//    screen_width = desktop.right;
//    screen_height = desktop.bottom;
//    resolution_factor = screen_height / 1080.0f;
//}
//
//// Вспомогательные функции
//float GetScopeFactor(float val) {
//    const float factors[] = { 1.0f, 1.2f, 3.84f, 7.68f, 0.8f };
//    return val * factors[std::clamp(scope, 0, 4)];
//}
//
//float GetBarrelFactor(float val) {
//    const float factors[] = { 1.0f, 0.8f, 0.5f, 0.9f };
//    return val * factors[std::clamp(barrel, 0, 3)];
//}
//
//float AdjustForFOVandSens(float val) {
//    float adjusted = val * (fov / 90.0f) * (1.0f / sensitivity) * resolution_factor;
//    adjusted = GetScopeFactor(adjusted);
//    adjusted = GetBarrelFactor(adjusted);
//    return adjusted;
//}
//
//void PreciseSleep(int ms) {
//    static LARGE_INTEGER frequency;
//    QueryPerformanceFrequency(&frequency);
//
//    LARGE_INTEGER start;
//    QueryPerformanceCounter(&start);
//
//    double target = start.QuadPart + (ms * frequency.QuadPart) / 1000.0;
//    LARGE_INTEGER current;
//
//    do {
//        QueryPerformanceCounter(&current);
//        if (current.QuadPart >= target) break;
//        Sleep(1);
//    } while (true);
//}
//
//// Основные функции
//void SmoothMove(float x, float y, float duration_ms) {
//    const int steps = std::max(10, static_cast<int>(duration_ms / 10));
//    const float step_time = duration_ms / steps;
//    const float step_x = x / steps;
//    const float step_y = y / steps;
//
//    float accum_x = 0.0f;
//    float accum_y = 0.0f;
//    float remainder_x = 0.0f;
//    float remainder_y = 0.0f;
//
//    for (int i = 0; i < steps; i++) {
//        if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000) || !(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
//            break;
//
//        accum_x += step_x + remainder_x;
//        accum_y += step_y + remainder_y;
//
//        int move_x = static_cast<int>(accum_x);
//        int move_y = static_cast<int>(accum_y);
//
//        remainder_x = accum_x - move_x;
//        remainder_y = accum_y - move_y;
//
//        accum_x -= move_x;
//        accum_y -= move_y;
//
//        if (move_x != 0 || move_y != 0) {
//            mouse_event(MOUSEEVENTF_MOVE, move_x, move_y, 0, 0);
//        }
//
//        PreciseSleep(static_cast<int>(step_time));
//    }
//}
//
//void HandleRecoilControl(const std::vector<Vector2>& pattern,
//    const std::vector<double>& control_times,
//    float base_delay) {
//    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
//    float rand_factor = randomization_level / 1000.0f;
//
//    for (size_t i = 0; i < pattern.size(); i++) {
//        if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000) || !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
//            is_recoil_active = false;
//            return;
//        }
//
//        float adjustedX = -pattern[i].x * (rcsX / 100.0f);
//        float adjustedY = -pattern[i].y * (rcsY / 100.0f);
//
//        adjustedX = AdjustForFOVandSens(adjustedX);
//        adjustedY = AdjustForFOVandSens(adjustedY);
//
//        // Применяем рандомизацию
//        adjustedX += adjustedX * rand_factor * dist(gen);
//        adjustedY += adjustedY * rand_factor * dist(gen);
//
//        // Динамическое время сглаживания
//        float control_time = (i < control_times.size()) ? control_times[i] : base_delay * 0.75f;
//        SmoothMove(adjustedX, adjustedY, control_time);
//
//        // Задержка между выстрелами
//        float sleep_time = base_delay - control_time;
//        if (sleep_time > 0) {
//            PreciseSleep(static_cast<int>(sleep_time));
//        }
//    }
//}
//
//void ProcessRecoilControl(int weapon_idx) {
//    if (recoil_enabled && (GetAsyncKeyState(VK_RBUTTON) & 0x8000) && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
//        if (!is_recoil_active) {
//            is_recoil_active = true;
//
//            std::vector<Vector2> pattern;
//            std::vector<double> control_times;
//            float delay = 100.0f;
//
//            switch (weapon_idx) {
//            case 1: // AK
//                pattern = Weapons::ak::patternak;
//                control_times = Weapons::ak::akcontrol;
//                delay = Weapons::ak::delay;
//                break;
//            case 2: // LR
//                pattern = Weapons::lr::patternlr;
//                control_times = Weapons::lr::controltime;
//                delay = Weapons::lr::delay;
//                break;
//                // ... другие оружия
//            default:
//                return;
//            }
//
//            HandleRecoilControl(pattern, control_times, delay);
//        }
//    }
//    else {
//        is_recoil_active = false;
//    }
//}
