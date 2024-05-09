#pragma once
#include "ArlineImgui.hpp"

namespace arline {

    inline auto Commands::begin() const noexcept -> v0
    {
        VkContext::AcquireNextImage();

        m.cmd = m.ctx->frames[m.ctx->frameIndex].commandBuffer;

        vkResetCommandPool(m.ctx->device, m.ctx->frames[m.ctx->frameIndex].commandPool, 0);

        auto const beginInfo{VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        }};

        vkBeginCommandBuffer(m.cmd, &beginInfo);
    }

    inline auto Commands::end() const noexcept -> v0
    {
        vkEndCommandBuffer(m.cmd);
        VkContext::PresentFrame();
    }

    inline auto Commands::beginPresent() const noexcept -> v0
    {
        auto imageBarrier{ VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .oldLayout = m.ctx->swapchainImages[m.ctx->imageIndex].layout,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m.ctx->swapchainImages[m.ctx->imageIndex].handle,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        }};

        if (imageBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        {
            imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

            m.ctx->swapchainImages[m.ctx->imageIndex].layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        } else
        {
            imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
            imageBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        }

        auto const dependency{ VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imageBarrier
        }};

        vkCmdPipelineBarrier2(m.cmd, &dependency);

        auto const colorAttachment{ VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m.ctx->swapchainImages[m.ctx->imageIndex].view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
        }};

        auto const renderingInfo{ VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .extent = m.ctx->swapchainExtent
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment
        }};

        vkCmdBeginRendering(m.cmd, &renderingInfo);
        vkCmdBindDescriptorSets(m.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m.ctx->pipelineLayout, 0, 1, &m.ctx->descriptorSet, 0, nullptr);

        {
            auto const viewport{ VkViewport{
                .y = static_cast<f32>(m.ctx->swapchainExtent.height),
                .width  =  static_cast<f32>(m.ctx->swapchainExtent.width),
                .height = -static_cast<f32>(m.ctx->swapchainExtent.height),
                .maxDepth = 1.f
            }};

            vkCmdSetViewport(m.cmd, 0, 1, &viewport);
        }
        {
            auto const scissor{ VkRect2D{
                .extent = m.ctx->swapchainExtent
            }};

            vkCmdSetScissor(m.cmd, 0, 1, &scissor);
        }
    }

    inline auto Commands::endPresent() const noexcept -> v0
    {
        vkCmdEndRendering(m.cmd);

        auto const imageBarrier{ VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_NONE,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m.ctx->swapchainImages[m.ctx->imageIndex].handle,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            },
        }};

        auto const dependency{ VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imageBarrier
        }};

        vkCmdPipelineBarrier2(m.cmd, &dependency);
    }

    inline auto Commands::setScissor(i32 x, i32 y, u32 width, u32 height) const noexcept -> v0
    {
        auto const scissor{ VkRect2D{
            .offset = {
                .x = x,
                .y = y
            },
            .extent = {
                .width = width,
                .height = height
            }
        }};

        vkCmdSetScissor(m.cmd, 0, 1, &scissor);
    }

    inline auto Commands::bindPipeline(Pipeline const& pipeline) const noexcept -> v0
    {
        vkCmdBindPipeline(m.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    inline auto Commands::draw(u32 vertexCount, u32 instanceCount, u32 vertex, u32 instance) const noexcept -> v0
    {
        vkCmdDraw(m.cmd, vertexCount, instanceCount, vertex, instance);
    }

    inline auto Commands::drawImGui() const noexcept -> v0
    {
        ImGui::Render();

        auto drawData{ ImGui::GetDrawData() };

        if (!drawData) return;

        auto vboSize{ drawData->TotalVtxCount * sizeof(ImDrawVert) };
        auto iboSize{ drawData->TotalIdxCount * sizeof(ImDrawIdx) };

        if (!vboSize) return;

        struct { u64 vbo, ibo; u32 textureID; }
        pushConstant{
            .vbo = *ImGuiContext::Get()->vbo.getAddress(),
            .ibo = *ImGuiContext::Get()->ibo.getAddress(),
            .textureID = ImGuiContext::Get()->fontTexture.getIndex()
        };

        this->bindPipeline(ImGuiContext::Get()->pipeline);
        this->pushConstant(&pushConstant);

        auto vtxOffset{ u32{} };
        auto idxOffset{ u32{} };
        auto vtxDrawOffset{ u32{} };
        auto idxDrawOffset{ u32{} };

        for (auto const cmdList : drawData->CmdLists)
        {
            auto const idxSize{ cmdList->IdxBuffer.Size * sizeof(ImDrawIdx) };
            auto const vtxSize{ cmdList->VtxBuffer.Size * sizeof(ImDrawVert) };

            ImGuiContext::Get()->ibo.write(cmdList->IdxBuffer.Data, idxSize, idxOffset);
            ImGuiContext::Get()->vbo.write(cmdList->VtxBuffer.Data, vtxSize, vtxOffset);

            idxOffset += idxSize;
            vtxOffset += vtxSize;
        
            for (auto const& cmd : cmdList->CmdBuffer)
            {
                this->setScissor(
                    static_cast<i32>(cmd.ClipRect.x), static_cast<i32>(cmd.ClipRect.y),
                    static_cast<u32>(cmd.ClipRect.z - cmd.ClipRect.x), static_cast<u32>(cmd.ClipRect.w - cmd.ClipRect.y)
                );
                this->draw(cmd.ElemCount, 1, cmd.IdxOffset + idxDrawOffset, cmd.VtxOffset + vtxDrawOffset);
            }

            idxDrawOffset += cmdList->IdxBuffer.Size;
            vtxDrawOffset += cmdList->VtxBuffer.Size;
        }
    }

    inline auto Commands::pushConstant(auto const* pData) const noexcept -> v0
    {
        static_assert(sizeof(*pData) <= 128);
        static constexpr auto stage{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };

        vkCmdPushConstants(
            m.cmd,
            VkContext::Get()->pipelineLayout,
            stage,
            0,
            sizeof(*pData),
            pData
        );
    }

    inline auto Commands::pushConstant(v0 const* pData, u32 dataSize) const noexcept -> v0
    {
        static constexpr auto stage{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };

        vkCmdPushConstants(
            m.cmd,
            VkContext::Get()->pipelineLayout,
            stage,
            0,
            dataSize,
            pData
        );
    }
}
