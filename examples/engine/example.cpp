#include <Arline.hpp>
#include <iostream>
#include <format>

struct Engine
{
    static ar::WindowInfo WindowInfo()
    {
        return ar::WindowInfo{
            .width = 1280,
            .height = 720,
            .minWidth = 100,
            .minHeight = 100,
            .maxWidth = 4000,
            .maxHeight = 2000,
            .title = "Hello World!"
        };
    }

    static ar::ContextInfo ContextInfo()
    {
        return ar::ContextInfo{
            .infoCallback = [](std::string_view message)
            { std::cout << std::format("INFO: {}\n", message); },
            .errorCallback = [](std::string_view message)
            { std::cerr << std::format("ERROR: {}\n", message); std::exit(EXIT_FAILURE); },
            .enableValidationLayers = true // VULKAN SDK is required
        };
    }

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
        commands.beginPresent(ar::RenderPass{
            .color = { 255, 50, 100, 255 },
            .loadOp = ar::LoadOp::eClear,
            .storeOp = ar::StoreOp::eStore
        });

        commands.drawImGui();

        commands.endPresent();
    }
};

int main()
{
    ar::InitEngine<Engine>();
}