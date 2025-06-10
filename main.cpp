// Dear ImGui: standalone example application for DirectX 11
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "global.h"
#include "custom.h"
#include "icon.h"
#include "KeyBinder.h"


// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);




// Main code
int main(int, char**)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,            // Скрыто из панели задач и можно сделать прозрачным
        wc.lpszClassName,
        L"Dear ImGui DirectX11 Example",
        WS_POPUP,                                    // Без рамки, без заголовка
        0, 0, 0, 0,                                  // Микро-размер, можно 0x0 тоже
        nullptr, nullptr, wc.hInstance, nullptr
    );

    //SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Загрузка шрифтов
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;

    // Основной шрифт — загружаем из файла
    ImFont* main_font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 16.0f);

    // Иконковый шрифт
    ImFontConfig icons_config;
    icons_config.MergeMode = false;
    icons_config.PixelSnapH = true;
    icons_config.FontDataOwnedByAtlas = false;
    icons_config.GlyphOffset.y = 1.0f;
    ImFont* icon_font = io.Fonts->AddFontFromMemoryTTF(rawData, rawDatasize, 21.0f, &icons_config);

    // Применяем шрифт по умолчанию для всего UI
    io.FontDefault = main_font;

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }


        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        //ТУТ МЕНЮ!!!!!!!!!!!
        {
            static bool anti_screenshot = false;
            static int selected_tab = 0;
            static bool recoil_enabled = true;
            static bool blatant_mode = false;
            static int weapon_idx = 0, scope_idx = 0, barrel_idx = 0;
            static int control_x = 100, control_y = 100;
            static int rand_x = 0, rand_y = 0;
            static bool weapon_detection = true;
            static int ads_mode = 0;
            static float sensitivity = 0.3f, ads_sensitivity = 1.0f;
            static int fov = 90;
            // Основное окно без рамок
            ImGui::SetNextWindowSize(ImVec2(700, 480), ImGuiCond_Once);
            ImGui::Begin("Resolution.club", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse);
            /*menu name*/
            {
                float time = ImGui::GetTime();

                // Цвет из твоего RGBA для "Resolution"
                ImVec4 baseColor = ImVec4(68 / 255.f, 23 / 255.f, 118 / 255.f, 1.0f);

                // Сделаем пульсацию яркости для "Resolution"
                float pulse = 0.5f + 0.5f * sinf(time * 3.0f); // меняется от 0 до 1

                ImVec4 resolutionColor = ImVec4(
                    baseColor.x * pulse,
                    baseColor.y * pulse,
                    baseColor.z * pulse,
                    1.0f);

                // Выводим "Resolution" с пульсирующим цветом
                ImGui::TextColored(resolutionColor, "Resolution");
                ImGui::SameLine();

                // Градиентное переливание для ".club"
                ImVec4 startColor = ImVec4(0.27f, 0.09f, 0.46f, 1.0f);
                ImVec4 endColor = ImVec4(0.0f, 0.8f, 0.7f, 1.0f);

                // Параметр t для плавного перехода
                float t = (sinf(time * 2.0f) + 1.0f) * 0.5f;

                ImVec4 clubColor = ImVec4(
                    startColor.x * (1.0f - t) + endColor.x * t,
                    startColor.y * (1.0f - t) + endColor.y * t,
                    startColor.z * (1.0f - t) + endColor.z * t,
                    1.0f);

                // Выводим ".club"
                ImGui::TextColored(clubColor, ".club");

            }
            ImGui::BeginChild("LeftTabs", ImVec2(60, 0), true);
            {
                //float window_width = ImGui::GetWindowSize().x;
                //float text_width = ImGui::CalcTextSize("A").x;
                //ImGui::SetCursorPosX((window_width - text_width) * 0.3f);

                //ImGui::PushFont(icon_font);
                //ImGui::TextColored(ImVec4(1, 1, 1, 1), "A");
                //ImGui::PopFont();

                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

                // Кнопка B
                ImGui::PushFont(icon_font);
                ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 0 ? ImVec4(0.0f, 0.0f, 0.0f, 0.0f) : ImVec4(0.1f, 0.1f, 0.1f, 0.15f));
                if (ImGui::Button("B", ImVec2(40, 40))) selected_tab = 0;
                ImGui::PopStyleColor();
                ImGui::PopFont();

                ImGui::Spacing();


                // Кнопка C
                ImGui::PushFont(icon_font);
                ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 1 ? ImVec4(0.0f, 0.0f, 0.0f, 0.0f) : ImVec4(0.1f, 0.1f, 0.1f, 0.15f));
                if (ImGui::Button("C", ImVec2(40, 40))) selected_tab = 1;
                ImGui::PopStyleColor();
                ImGui::PopFont();

                ImGui::PopStyleVar();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            // ===== Правая часть (контент) =====
            ImGui::BeginChild("RightContent", ImVec2(0, 0), true);
            {
                if (selected_tab == 0)
                {
                    ImGui::Text("Main");

                    ImGui::Text("Recoil"); ImGui::SameLine();
                    CustomWidgets::ToggleSwitch("##RecoilSwitch", &recoil_enabled);

                    ImGui::SameLine(0, 30.0f);
                    ImGui::Text("Blatant Mode"); ImGui::SameLine();
                    CustomWidgets::ToggleSwitch("##BlatantSwitch", &blatant_mode);

                    const char* weapons[] = { "Custom", "AK-47", "M4A4" };
                    CustomWidgets::ComboCs("Selected Weapon", &weapon_idx, weapons, IM_ARRAYSIZE(weapons));

                    const char* scopes[] = { "Holosight", "No Scope" };
                    CustomWidgets::ComboCs("Selected Scope", &scope_idx, scopes, IM_ARRAYSIZE(scopes));

                    const char* barrels[] = { "No Barrel", "Suppressor", "Long Barrel" };
                    CustomWidgets::ComboCs("Selected Barrel", &barrel_idx, barrels, IM_ARRAYSIZE(barrels));

                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    float child_width = ImGui::GetContentRegionAvail().x * 0.48f;

                    ImGui::BeginChild("TweaksPanel", ImVec2(child_width, 0), true);
                    {
                        ImGui::Text("Tweaks");
                        ImGui::SliderInt("Control Amount X", &control_x, 0, 100);
                        ImGui::SliderInt("Control Amount Y", &control_y, 0, 100);
                        ImGui::SliderInt("Randomisation X", &rand_x, 0, 100);
                        ImGui::SliderInt("Randomisation Y", &rand_y, 0, 100);
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::BeginChild("MiscPanel", ImVec2(0, 0), true);
                    {
                        ImGui::Text("Misc");
                        ImGui::Text("Weapon Detection"); ImGui::SameLine();
                        CustomWidgets::ToggleSwitch("##WeaponDetect", &weapon_detection);
                      //  ImGui::Checkbox("Anti Screenshot", &anti_screenshot);
                        ImVec4 gradient_color = ImVec4(0.0f, 0.1f, 0.1f, 0.1f);  // голубой цвет

                  //      CustomWidgets::checkboxCs("Anti Screenshot", &anti_screenshot, gradient_color);
                        ImGui::Text("Anti Screenshot"); ImGui::SameLine();
                        CustomWidgets::ToggleSwitch("##Anti Screenshot", &anti_screenshot);
                        const char* ads_modes[] = { "No Hipfire", "Hipfire Enabled" };
                        CustomWidgets::ComboCs("ADS Mode", &ads_mode, ads_modes, IM_ARRAYSIZE(ads_modes));
                        ImGui::SliderFloat("Sensitivity", &sensitivity, 0.0f, 1.0f, "%.3f");
                        ImGui::SliderFloat("ADS Sensitivity", &ads_sensitivity, 0.0f, 2.0f, "%.3f");
                        ImGui::SliderInt("FOV", &fov, 70, 120);

                    }
                    ImGui::EndChild();
                }
                else if (selected_tab == 1)
                {
                    ImGui::Text("Keybinds");
                    static int keybindIndex = 0;

            
                }
            }
            ImGui::EndChild();
            ImGui::End();



            // === Anti Screenshot логика ===
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
                for (int i = 0; i < platform_io.Viewports.Size; i++)
                {
                    HWND hwnd_view = (HWND)platform_io.Viewports[i]->PlatformHandleRaw;
                    SetWindowDisplayAffinity(hwnd_view, anti_screenshot ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
                }
            }


        }
        //ТУТ КОНЕЦ МЕНЮ ВЫБЛЯДОК!!!!!!!!!!!
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f,0.0f,0.0f,0.0f  };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);  // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
