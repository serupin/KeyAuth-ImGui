#include "ui.hh"
#include "../globals.hh"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

void Login()
{
    KeyAuthApp.login(globals.username, globals.password);

    if (KeyAuthApp.data.success)
    {
        for (std::string value : KeyAuthApp.data.subscriptions)globals.subs += value + " ";
        globals.tab = 1;
    }
}

void Register()
{
    KeyAuthApp.regstr(globals.username, globals.password, globals.key);

    if (KeyAuthApp.data.success)
    {
        for (std::string value : KeyAuthApp.data.subscriptions)globals.subs += value + " ";
        globals.tab = 1;
    }
}

void Upgrade()
{
    KeyAuthApp.upgrade(globals.username, globals.key);

    if (KeyAuthApp.data.success)
    {
        for (std::string value : KeyAuthApp.data.subscriptions)globals.subs += value + " ";
        globals.tab = 1;
    }
}

void License()
{
    KeyAuthApp.license(globals.key);

    if (KeyAuthApp.data.success)
    {
        for (std::string value : KeyAuthApp.data.subscriptions)globals.subs += value + " ";
        globals.tab = 1;
    }
}

void ui::render() {
    if (!globals.active) return;

    switch (globals.tab)
    {
        break;

    case 0:
    {
        ImGui::SetNextWindowPos(ImVec2(window_pos.x, window_pos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));
        ImGui::SetNextWindowBgAlpha(1.0f);

        ImGui::Begin(window_title, &globals.active, window_flags);
        {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Login"))
                {
                    ImGui::Text("Login using username and password.");

                    ImGui::Separator();

                    ImGui::InputText("Username", globals.username, IM_ARRAYSIZE(globals.username));
                    ImGui::InputText("Password", globals.password, IM_ARRAYSIZE(globals.password), ImGuiInputTextFlags_Password);
                    if (ImGui::Button("Login")) {
                        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Login, NULL, NULL, NULL);
                    }


                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Register"))
                {
                    ImGui::Text("Register using username, password and key.");

                    ImGui::Separator();

                    ImGui::InputText("Username", globals.username, IM_ARRAYSIZE(globals.username));
                    ImGui::InputText("Password", globals.password, IM_ARRAYSIZE(globals.password), ImGuiInputTextFlags_Password);
                    ImGui::InputText("Key", globals.key, IM_ARRAYSIZE(globals.key));
                    if (ImGui::Button("Register")) {
                        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Register, NULL, NULL, NULL);
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Upgrade"))
                {
                    ImGui::Text("Upgrade your account using key.");

                    ImGui::Separator();

                    ImGui::InputText("Username", globals.username, IM_ARRAYSIZE(globals.username));
                    ImGui::InputText("Key", globals.key, IM_ARRAYSIZE(globals.key));
                    if (ImGui::Button("Upgrade")) {
                        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Upgrade, NULL, NULL, NULL);
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("License Key"))
                {
                    ImGui::Text("Login using key only.");
                    
                    ImGui::Separator();

                    ImGui::InputText("Key", globals.key, IM_ARRAYSIZE(globals.key));
                    if (ImGui::Button("Login")) {
                        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)License, NULL, NULL, NULL);
                    }

                    ImGui::EndTabItem();
                }


            }
            ImGui::EndTabBar();
            ImGui::SetCursorPosY(280);
            ImGui::Separator();

            ImGui::Text(KeyAuthApp.data.message.c_str());
        }
        ImGui::End();
    }
    case 1:
    {
        ImGui::SetNextWindowPos(ImVec2(window_pos.x, window_pos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2({ 500, 300 }));
        ImGui::SetNextWindowBgAlpha(1.0f);

        ImGui::Begin(window_title, &globals.active, window_flags);
        {
            ImGui::Text("User Data");
            ImGui::Separator();

            ImGui::Text("Username : %s", KeyAuthApp.data.username.c_str());
            ImGui::Text("IP address : %s", KeyAuthApp.data.ip.c_str());
            ImGui::Text("HWID : %s", KeyAuthApp.data.hwid.c_str());
            ImGui::Text("Create Date : %s", globals.CreateDate.c_str());
            ImGui::Text("Last Login : %s", globals.LastLogin.c_str());
            ImGui::Text("Subscription Names : %s", globals.subs.c_str());
            ImGui::Text("Subscription Expiry : %s", globals.Expiry.c_str());


            if (ImGui::Button("Back"))
            {
                globals.tab = 0;
            }
            ImGui::SetCursorPosY(280);
            ImGui::Separator();

            ImGui::Text(KeyAuthApp.data.message.c_str());
           
        }
        ImGui::End();
    }
    }

    
}

void ui::init(LPDIRECT3DDEVICE9 device) {
    dev = device;
	
    // colors
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_TitleBg] = ImColor(30, 30, 45);
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(30, 30, 45);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(30, 30, 45);

    style.Colors[ImGuiCol_WindowBg] = ImColor(21, 21, 33);

    style.Colors[ImGuiCol_Tab] = ImColor(24, 125, 228);
    style.Colors[ImGuiCol_TabActive] = ImColor(54, 153, 255);
    style.Colors[ImGuiCol_TabHovered] = ImColor(24, 125, 228);
    style.Colors[ImGuiCol_Button] = ImColor(54, 153, 255);


	if (window_pos.x == 0) {
		RECT screen_rect{};
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		screen_res = ImVec2(float(screen_rect.right), float(screen_rect.bottom));
		window_pos = (screen_res - window_size) * 0.5f;

		// init images here
	}
}