#include <Arline.hpp>
#include <format>
#include <cmath>

using namespace ar::types;

struct Engine
{
    static consteval u32 UseImgui() { return 1; }

    inline auto update() noexcept -> v0
    {
        ar::Window::PollEvents();
    }

    inline auto renderGui() noexcept -> v0
    {
        ImGui::ShowDemoWindow();
    }

    inline auto recordCommands(ar::Commands const& commands) noexcept -> v0
    {
        commands.beginPresent();
        commands.drawImGui();
        commands.endPresent();
    }
};

auto main() -> i32
{
    ar::Context{
        ar::WindowInfo{
            .width = 1280,
            .height = 720,
            .minWidth = 400,
            .minHeight = 300,
            .title = "Example - ImGui"
        },
        ar::ContextInfo{
            .infoCallback = [](std::string_view message) { std::printf("INFO: %s\n", message.data()); },
            .errorCallback = [](std::string_view message) { std::printf("ERROR: %s\n", message.data()); exit(1); },
            .enableValidationLayers = true
        }
    }.initEngine(Engine{});
}