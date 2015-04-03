#include "UI.h"
#include "imgui.h"
#include "libENUgraphics\renderer.h"
#include "imgui_impl_glfw_gl3.h"
#include <GLFW/glfw3.h>

static void ShowExampleAppFixedOverlay(bool opened)
{
  ImGui::SetNextWindowPos(ImVec2(10, 10));
  if (!ImGui::Begin("Overlay", &opened, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
  {
    ImGui::End();
    return;
  }

  ImGui::Text("Graphics Demo - Sam Serrels\nPress [M] to show  Menu\nPress [N] to toggle this display");
  ImGui::Separator();
  ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();
}





void initUI(){
  ImGui_ImplGlfwGL3_Init(graphics_framework::renderer::get_window(), true);
}

void UpdateUI()
{
  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplGlfwGL3_NewFrame();
  ImVec4 clear_color = ImColor(114, 144, 154);

  // 1. Show a simple window
  // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
  {
    bool show_test_window = true;
    bool show_another_window = false;

    static float f = 0.0f;
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::ColorEdit3("clear color", (float*)&clear_color);
    if (ImGui::Button("Test Window")) show_test_window ^= 1;
    if (ImGui::Button("Another Window")) show_another_window ^= 1;

  }
  ShowExampleAppFixedOverlay(true);

}

void DrawUI()
{
  ImGui::Render();
}

void ShutDownUI()
{
  ImGui_ImplGlfwGL3_Shutdown();
}