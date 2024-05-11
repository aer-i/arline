#include <Arline.hpp>

struct Engine
{
    void update()
    {
        ar::Window::PollEvents();
    }

    void renderGui()
    {
        ImGui::ShowDemoWindow();
    }

    void recordCommands(ar::Commands const& commands)
    {
        commands.beginPresent();
        commands.drawImGui();
        commands.endPresent();
    }
};

int main()
{
    ar::InitEngine<Engine>();
}