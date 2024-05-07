#pragma once

namespace arline {

    inline auto Commands::begin() const noexcept -> v0
    {
        VkContext::AcquireNextImage();

        m.cmd = m.ctx->currentFrame->commandBuffer;

        vkResetCommandPool(m.ctx->device, m.ctx->currentFrame->commandPool, 0);

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

        auto const dependency{VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imageBarrier
        }};

        vkCmdPipelineBarrier2(m.cmd, &dependency);

        auto const colorAttachment{VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m.ctx->swapchainImages[m.ctx->imageIndex].view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {
                .color = VkClearColorValue{
                    .float32 = {0.5f, 0.25f, 0.75f, 1.f}
                }
            }
        }};

        auto const renderingInfo{VkRenderingInfo{
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
            auto const viewport{VkViewport{
                .y = static_cast<f32>(m.ctx->swapchainExtent.height),
                .width  =  static_cast<f32>(m.ctx->swapchainExtent.width),
                .height = -static_cast<f32>(m.ctx->swapchainExtent.height),
                .maxDepth = 1.f
            }};

            vkCmdSetViewport(m.cmd, 0, 1, &viewport);
        }
        {
            auto const scissor{VkRect2D{
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

    inline auto Commands::bindPipeline(Pipeline const& pipeline) const noexcept -> v0
    {
        vkCmdBindPipeline(m.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    inline auto Commands::draw(u32 vertexCount) const noexcept -> v0
    {
        vkCmdDraw(m.cmd, vertexCount, 1, 0, 0);
    }
}