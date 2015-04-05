#include "UI.h"
#include "imgui.h"
#include "libENUgraphics\renderer.h"
#include "imgui_impl_glfw_gl3.h"
#include <GLFW/glfw3.h>
#include "Enviroment.h"

static bool showMenu;

static void ShowStats(bool opened)
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

static void Menu(){
  // 1. Show a simple window
  // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    bool show_test_window = true;
    static bool showTestWindow = false;
    static bool time = false;
    static float speed = false;

    bool opened;
    if (!ImGui::Begin("Menu", &opened, ImVec2(550, 680)))
    {
      // Early out if the window is collapsed, as an optimization.
      ImGui::End();
      return;
    }

    static float f = 0.0f;
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    if (ImGui::Button("Test Window")) show_test_window ^= 1;
    if (ImGui::Button("Show Test Window")) showTestWindow ^= 1;
    
    if (ImGui::CollapsingHeader("Enviroment"))
    {
        ImGui::Checkbox("Enable Time", &time); 
        ImGui::SliderFloat("Day/Night Speed", &speed, -10.0f, 10.0f, "%.4f", 3.0f); 
        ImGui::SameLine();
        if (ImGui::Button("Reset")) speed = 0;
        ImGui::Text("Dayscale: %f , %i", Enviroment::dayscale, Enviroment::daymode);
    }

    if (showTestWindow) ImGui::ShowTestWindow();
    ImGui::End();
}



void UpdateUI()
{
  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplGlfwGL3_NewFrame();

  static int keystate;
  int newkeystate = glfwGetKey(graphics_framework::renderer::get_window(), 'M');
  if (newkeystate && newkeystate != keystate){
    showMenu = !showMenu;
    if (showMenu){
      glfwSetInputMode(graphics_framework::renderer::get_window(), GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    }
    else{
      glfwSetInputMode(graphics_framework::renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }
  keystate = newkeystate;


  ShowStats(true);

  if (showMenu){
    Menu();
  }

}

void initUI(){
  ImGui_ImplGlfwGL3_Init(graphics_framework::renderer::get_window(), true);
  showMenu = false;
}

void DrawUI()
{
  ImGui::Render();
}

void ShutDownUI()
{
  ImGui_ImplGlfwGL3_Shutdown();
}