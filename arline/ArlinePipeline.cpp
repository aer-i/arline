#include "ArlinePipeline.hpp"

arline::Pipeline::Pipeline() noexcept
    : p{}
{}

arline::Pipeline::~Pipeline() noexcept
{
    if (p.handle)
    {
        vkDestroyPipeline(VkContext::Get()->device, p.handle, nullptr);
    }
}

arline::Pipeline::Pipeline(Pipeline&& other) noexcept
    : p{ other.p }
{
    other.p = {};
}

auto arline::Pipeline::operator=(Pipeline&& other) noexcept -> Pipeline&
{
    this->~Pipeline();
    this->p = other.p;
    other.p = {};

    return *this;
}

arline::GraphicsPipeline::GraphicsPipeline(Config&& config) noexcept
{
    auto shaders{ std::vector<VkPipelineShaderStageCreateInfo>{} };
    shaders.reserve(config.shaders.size());

    for (auto& shader : config.shaders)
    {
        shaders.emplace_back(shader);
    }

    auto constexpr dynamicStates{ std::array{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR} };

    auto const dynamicStateCreateInfo{ VkPipelineDynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<u32>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    }};

    auto const viewportStateCreateInfo{ VkPipelineViewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1
    }};

    auto const inputAssemblyStateCreateInfo{ VkPipelineInputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = static_cast<VkPrimitiveTopology>(config.topology)
    }};

    auto const rasterizationStateCreateInfo{ VkPipelineRasterizationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = static_cast<VkPolygonMode>(config.polygonMode),
        .cullMode = static_cast<VkCullModeFlags>(config.cullMode),
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f
    }};

    auto const multisampleStateCreateInfo{ VkPipelineMultisampleStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    }};

    auto const blendAttachmentState{ VkPipelineColorBlendAttachmentState{
        .blendEnable = static_cast<b8>(config.flags & FlagBits::eColorBlending),
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    }};

    auto const colorBlendStateCreateInfo{ VkPipelineColorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &blendAttachmentState
    }};

    auto const depthStencilStateCreateInfo{ VkPipelineDepthStencilStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = static_cast<b8>(config.flags & FlagBits::eDepthTest),
        .depthWriteEnable = static_cast<b8>(config.flags & FlagBits::eDepthWrite),
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencilTestEnable = false
    }};

    auto const vertexInputStateCreateInfo{ VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    }};

    auto const renderingCreateInfo{ VkPipelineRenderingCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &VkContext::Get()->colorFormat,
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    }};

    auto const pipelineCreateInfo{ VkGraphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCreateInfo,
        .stageCount = static_cast<u32>(shaders.size()),
        .pStages = shaders.data(),
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = &depthStencilStateCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = VkContext::Get()->pipelineLayout
    }};

    p.handle = VkContext::CreatePipeline(&pipelineCreateInfo);
}
