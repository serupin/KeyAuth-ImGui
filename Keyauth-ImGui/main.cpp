#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "auth/auth.hpp"
std::string tm_to_readable_time(tm ctx);
static std::time_t string_to_timet(std::string timestamp);
static std::tm timet_to_tm(time_t timestamp);

using namespace KeyAuth;

std::string name = skCrypt("").decrypt();
std::string ownerid = skCrypt("").decrypt();
std::string version = skCrypt("").decrypt();
std::string url = skCrypt("https://keyauth.win/api/1.3/").decrypt(); // change if using KeyAuth custom domains feature
std::string path = skCrypt("").decrypt(); //optional, set a path if you're using the token validation setting

api KeyAuthApp(name, ownerid, version, url, path);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;
RECT rc;
ImVec2 WSize = { 500, 300 };

int tab = 0;
int subtab = 0;

static char buf1[256] = "";
static char buf2[256] = "";
static char buf3[256] = "";


std::string GetReadableTimeDifference(std::string unixTimestamp) {
    std::time_t expiry = std::stol(unixTimestamp);
    std::time_t now = std::time(0);
    std::time_t diff = expiry - now;

    if (diff <= 0) {
        return "Expired";
    }

    int days = diff / (60 * 60 * 24);

    if (days > 365) {
        return "Lifetime";
    }

    int hours = (diff % (60 * 60 * 24)) / (60 * 60);
    int minutes = (diff % (60 * 60)) / 60;
    int seconds = diff % 60;

    std::string result;

    if (days > 0) {
        result += std::to_string(days) + " day(s)";
    }
    if (hours > 0) {
        if (!result.empty()) result += ", ";
        result += std::to_string(hours) + " hour(s)";
    }
    if (minutes > 0) {
        if (!result.empty()) result += ", ";
        result += std::to_string(minutes) + " minute(s)";
    }
    if (seconds > 0) {
        if (!result.empty()) result += ", ";
        result += std::to_string(seconds) + " second(s)";
    }

    return result.empty() ? "Expired" : result;
}

std::string GetReadableTimeAgo(std::string unixTimestamp) {
    std::time_t eventTime = std::stol(unixTimestamp);
    std::time_t now = std::time(0);
    std::time_t diff = now - eventTime;

    if (diff <= 0) {
        return "Just now";
    }

    int days = diff / (60 * 60 * 24);
    int hours = (diff % (60 * 60 * 24)) / (60 * 60);
    int minutes = (diff % (60 * 60)) / 60;
    int seconds = diff % 60;

    std::string result;

    if (days > 0) {
        result += std::to_string(days) + " day(s)";
    }
    if (hours > 0) {
        if (!result.empty()) result += ", ";
        result += std::to_string(hours) + " hour(s)";
    }
    if (minutes > 0) {
        if (!result.empty()) result += ", ";
        result += std::to_string(minutes) + " minute(s)";
    }
    if (seconds > 0) {
        if (!result.empty()) result += ", ";
        result += std::to_string(seconds) + " second(s)";
    }

    return result.empty() ? "Just now" : result + " ago";
}







int main(int, char**)
{
    KeyAuthApp.init();
    if (!KeyAuthApp.response.success)
    {
        std::cout << skCrypt("\n Status: ") << KeyAuthApp.response.message;
        Sleep(1500);
        exit(1);
    }

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Keyauth", NULL };
    ::RegisterClassExW(&wc);
    hwnd = CreateWindowEx(NULL, "Keyauth", "Keyauth", WS_POPUP | WS_EX_TOPMOST, (GetSystemMetrics(SM_CXSCREEN) / 2) - (WSize.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (WSize.y / 2), WSize.x, WSize.y, 0, 0, 0, 0);
    SetWindowRgn(hwnd, CreateRoundRectRgn(0, 0, WSize.x, WSize.y, 20, 20), TRUE);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImFont* Text = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    ImFont* Title = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 35.0f);
    

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    style.FrameRounding = 5.f;
    style.Colors[ImGuiCol_ChildBg] = ImColor(15, 15, 23);
    style.Colors[ImGuiCol_Button] = ImColor(15, 15, 23);
    style.Colors[ImGuiCol_ButtonActive] = ImColor(21, 57, 140);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(21, 57, 140);



    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(9, 9, 13);

    bool done = false;
    while (!done)
    {
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

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowSize({ WSize.x, WSize.y });
            ImGui::Begin("Window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
            {
                GetWindowRect(hwnd, &rc);

                ImVec2 windowPos = ImGui::GetWindowPos();
                if (windowPos.x != 0 || windowPos.y != 0)
                {
                    MoveWindow(hwnd, rc.left + windowPos.x, rc.top + windowPos.y, WSize.x, WSize.y, TRUE);
                    ImGui::SetWindowPos(ImVec2(0, 0));
                }

                if (tab == 0)
                {
                    ImGui::SetCursorPos({ 0,0 });
                    ImGui::BeginChild("Navbar", { 100, WSize.y });
                    {
                        ImGui::Indent(10);

                        if (ImGui::Button("Login", { 80, 25 }))
                            subtab = 0;

                        if (ImGui::Button("Register", { 80, 25 }))
                            subtab = 1;

                        if (ImGui::Button("Upgrade", { 80, 25 }))
                            subtab = 2;

                        if (ImGui::Button("License", { 80, 25 }))
                            subtab = 3;


                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos({ 110, 10 });
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.f);
                    ImGui::BeginChild("Frame", { 380, 280 });
                    {
                        if (subtab == 0) {
                            ImGui::PushFont(Title);  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Login").x) / 2.0f); ImGui::Text("Login"); ImGui::PopFont();

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);
                            ImGui::PushItemWidth(200.0f);
                            ImGui::InputTextWithHint("##Username", "Username", buf1, sizeof(buf1));

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);
                            ImGui::InputTextWithHint("##Password", "Password", buf2, sizeof(buf2), ImGuiInputTextFlags_Password);

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 100.0f) / 2.0f);

                            if (ImGui::Button("Submit", ImVec2(100.0f, 0))) {
                                KeyAuthApp.login(buf1, buf2);
                                if (KeyAuthApp.response.success)
                                {
                                    tab = 1;
                                }
                            }
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(KeyAuthApp.response.message.c_str()).x) / 2.0f); ImGui::Text(KeyAuthApp.response.message.c_str());
                        }

                        if (subtab == 1) {
                            ImGui::PushFont(Title);  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Register").x) / 2.0f); ImGui::Text("Register"); ImGui::PopFont();

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);
                            ImGui::PushItemWidth(200.0f);
                            ImGui::InputTextWithHint("##Username", "Username", buf1, sizeof(buf1));

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);
                            ImGui::InputTextWithHint("##Password", "Password", buf2, sizeof(buf2), ImGuiInputTextFlags_Password);

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);
                            ImGui::InputTextWithHint("##Key", "Key", buf3, sizeof(buf3));

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 100.0f) / 2.0f);
                            if (ImGui::Button("Submit", ImVec2(100.0f, 0))) {
                                KeyAuthApp.regstr(buf1, buf2, buf3);
                                if (KeyAuthApp.response.success)
                                {
                                    tab = 1;
                                }
                            }
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(KeyAuthApp.response.message.c_str()).x) / 2.0f); ImGui::Text(KeyAuthApp.response.message.c_str());
                        }

                        if (subtab == 2) {
                            ImGui::PushFont(Title);  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Upgrade").x) / 2.0f); ImGui::Text("Upgrade"); ImGui::PopFont();

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);
                            ImGui::PushItemWidth(200.0f);
                            ImGui::InputTextWithHint("##Username", "Username", buf1, sizeof(buf1));

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);
                            ImGui::InputTextWithHint("##Key", "Key", buf3, sizeof(buf3));

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 100.0f) / 2.0f);
                            if (ImGui::Button("Submit", ImVec2(100.0f, 0))) {
                                KeyAuthApp.upgrade(buf1, buf3);
                            }
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(KeyAuthApp.response.message.c_str()).x) / 2.0f); ImGui::Text(KeyAuthApp.response.message.c_str());
                        }

                        if (subtab == 3) {
                            ImGui::PushFont(Title);  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("License").x) / 2.0f); ImGui::Text("License"); ImGui::PopFont();

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 200.0f) / 2.0f);

                            ImGui::PushItemWidth(200.0f);

                            ImGui::InputTextWithHint("##Key", "Key", buf3, sizeof(buf3));

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 100.0f) / 2.0f);
                            if (ImGui::Button("Submit", ImVec2(100.0f, 0))) {
                                KeyAuthApp.license(buf3);
                                if (KeyAuthApp.response.success)
                                {
                                    tab = 1;
                                }
                            }
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(KeyAuthApp.response.message.c_str()).x) / 2.0f); ImGui::Text(KeyAuthApp.response.message.c_str());
                        }
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar(1);
                }
                if (tab == 1) {
                    ImGui::SetCursorPos({ 0,0 });
                    ImGui::BeginChild("Navbar", { 100, WSize.y });
                    {
                        ImGui::Indent(10);

                        if (ImGui::Button("User Data", { 80, 25 }))
                            subtab = 0;

                       


                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos({ 110, 10 });
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.f);
                    ImGui::BeginChild("Frame", { 380, 280 });
                    {
                        if (subtab == 0) {
                            ImGui::PushFont(Title);  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("User Data").x) / 2.0f); ImGui::Text("User Data"); ImGui::PopFont();

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(KeyAuthApp.user_data.username.c_str()).x) / 2.0f); ImGui::Text(KeyAuthApp.user_data.username.c_str());
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(KeyAuthApp.user_data.ip.c_str()).x) / 2.0f); ImGui::Text(KeyAuthApp.user_data.ip.c_str());
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(KeyAuthApp.user_data.hwid.c_str()).x) / 2.0f); ImGui::Text(KeyAuthApp.user_data.hwid.c_str());
                            
                            std::string CreationDate = "Creation Date : " + GetReadableTimeAgo(KeyAuthApp.user_data.createdate);
                            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(CreationDate.c_str()).x) / 2.0f);
                            ImGui::Text(CreationDate.c_str());

                           
                            std::string LastLogin = "Last Login : " + GetReadableTimeAgo(KeyAuthApp.user_data.lastlogin);
                            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(LastLogin.c_str()).x) / 2.0f);
                            ImGui::Text(LastLogin.c_str());

                            ImGui::Separator();
                            ImGui::PushFont(Title);  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Subscriptions").x) / 2.0f); ImGui::Text("Subscriptions"); ImGui::PopFont();
                            for (int i = 0; i < KeyAuthApp.user_data.subscriptions.size(); i++) {
                                auto sub = KeyAuthApp.user_data.subscriptions.at(i);

                                std::string Sub = sub.name + " : " + GetReadableTimeDifference(sub.expiry.c_str());

                                ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(Sub.c_str()).x) / 2.0f);
                                ImGui::Text(Sub.c_str());

                            }

                           
                        }

                        
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar(1);
                }
            }
            ImGui::End();
        }
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}


bool CreateDeviceD3D(HWND hWnd)
{
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

std::string tm_to_readable_time(tm ctx) {
    char buffer[80];

    strftime(buffer, sizeof(buffer), "%a %m/%d/%y %H:%M:%S %Z", &ctx);

    return std::string(buffer);
}

static std::time_t string_to_timet(std::string timestamp) {
    auto cv = strtol(timestamp.c_str(), NULL, 10); // long

    return (time_t)cv;
}

static std::tm timet_to_tm(time_t timestamp) {
    std::tm context;

    localtime_s(&context, &timestamp);

    return context;
}