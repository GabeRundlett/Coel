#define NOMINMAX
#include <coel/vulkan/core.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <iostream>

constexpr TBuiltInResource DEFAULT_SHADER_RESOURCE_SIZES = TBuiltInResource{
    .maxLights = 32,
    .maxClipPlanes = 6,
    .maxTextureUnits = 32,
    .maxTextureCoords = 32,
    .maxVertexAttribs = 64,
    .maxVertexUniformComponents = 4096,
    .maxVaryingFloats = 64,
    .maxVertexTextureImageUnits = 1 << 16,
    .maxCombinedTextureImageUnits = 1 << 16,
    .maxTextureImageUnits = 1 << 16,
    .maxFragmentUniformComponents = 4096,
    .maxDrawBuffers = 32,
    .maxVertexUniformVectors = 128,
    .maxVaryingVectors = 8,
    .maxFragmentUniformVectors = 16,
    .maxVertexOutputVectors = 16,
    .maxFragmentInputVectors = 15,
    .minProgramTexelOffset = -8,
    .maxProgramTexelOffset = 7,
    .maxClipDistances = 8,
    .maxComputeWorkGroupCountX = 65535,
    .maxComputeWorkGroupCountY = 65535,
    .maxComputeWorkGroupCountZ = 65535,
    .maxComputeWorkGroupSizeX = 1024,
    .maxComputeWorkGroupSizeY = 1024,
    .maxComputeWorkGroupSizeZ = 64,
    .maxComputeUniformComponents = 1024,
    .maxComputeTextureImageUnits = 1 << 16,
    .maxComputeImageUniforms = 1 << 16,
    .maxComputeAtomicCounters = 8,
    .maxComputeAtomicCounterBuffers = 1,
    .maxVaryingComponents = 60,
    .maxVertexOutputComponents = 64,
    .maxGeometryInputComponents = 64,
    .maxGeometryOutputComponents = 128,
    .maxFragmentInputComponents = 128,
    .maxImageUnits = 1 << 16,
    .maxCombinedImageUnitsAndFragmentOutputs = 8,
    .maxCombinedShaderOutputResources = 8,
    .maxImageSamples = 0,
    .maxVertexImageUniforms = 0,
    .maxTessControlImageUniforms = 0,
    .maxTessEvaluationImageUniforms = 0,
    .maxGeometryImageUniforms = 0,
    .maxFragmentImageUniforms = 8,
    .maxCombinedImageUniforms = 8,
    .maxGeometryTextureImageUnits = 16,
    .maxGeometryOutputVertices = 256,
    .maxGeometryTotalOutputComponents = 1024,
    .maxGeometryUniformComponents = 1024,
    .maxGeometryVaryingComponents = 64,
    .maxTessControlInputComponents = 128,
    .maxTessControlOutputComponents = 128,
    .maxTessControlTextureImageUnits = 16,
    .maxTessControlUniformComponents = 1024,
    .maxTessControlTotalOutputComponents = 4096,
    .maxTessEvaluationInputComponents = 128,
    .maxTessEvaluationOutputComponents = 128,
    .maxTessEvaluationTextureImageUnits = 16,
    .maxTessEvaluationUniformComponents = 1024,
    .maxTessPatchComponents = 120,
    .maxPatchVertices = 32,
    .maxTessGenLevel = 64,
    .maxViewports = 16,
    .maxVertexAtomicCounters = 0,
    .maxTessControlAtomicCounters = 0,
    .maxTessEvaluationAtomicCounters = 0,
    .maxGeometryAtomicCounters = 0,
    .maxFragmentAtomicCounters = 8,
    .maxCombinedAtomicCounters = 8,
    .maxAtomicCounterBindings = 1,
    .maxVertexAtomicCounterBuffers = 0,
    .maxTessControlAtomicCounterBuffers = 0,
    .maxTessEvaluationAtomicCounterBuffers = 0,
    .maxGeometryAtomicCounterBuffers = 0,
    .maxFragmentAtomicCounterBuffers = 1,
    .maxCombinedAtomicCounterBuffers = 1,
    .maxAtomicCounterBufferSize = 16384,
    .maxTransformFeedbackBuffers = 4,
    .maxTransformFeedbackInterleavedComponents = 64,
    .maxCullDistances = 8,
    .maxCombinedClipAndCullDistances = 8,
    .maxSamples = 4,
    .maxMeshOutputVerticesNV = 256,
    .maxMeshOutputPrimitivesNV = 512,
    .maxMeshWorkGroupSizeX_NV = 32,
    .maxMeshWorkGroupSizeY_NV = 1,
    .maxMeshWorkGroupSizeZ_NV = 1,
    .maxTaskWorkGroupSizeX_NV = 32,
    .maxTaskWorkGroupSizeY_NV = 1,
    .maxTaskWorkGroupSizeZ_NV = 1,
    .maxMeshViewCountNV = 4,
    .limits{
        .nonInductiveForLoops = 1,
        .whileLoops = 1,
        .doWhileLoops = 1,
        .generalUniformIndexing = 1,
        .generalAttributeMatrixVectorIndexing = 1,
        .generalVaryingIndexing = 1,
        .generalSamplerIndexing = 1,
        .generalVariableIndexing = 1,
        .generalConstantMatrixVectorIndexing = 1,
    },
};

namespace coel::vulkan {
    GraphicsPipeline::GraphicsPipeline(const Config &config) {
        device_handle = config.device_handle;
        const VkPipelineLayoutCreateInfo pipeline_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .setLayoutCount = static_cast<uint32_t>(config.descriptor_set_layouts.size()),
            .pSetLayouts = config.descriptor_set_layouts.data(),
        };
        vkCreatePipelineLayout(config.device_handle, &pipeline_layout_ci, nullptr, &layout);
        constexpr auto NUM_DYNAMIC_STATES = 2;
        VkGraphicsPipelineCreateInfo pipeline_ci;
        VkPipelineCacheCreateInfo pipelineCache;
        VkPipelineVertexInputStateCreateInfo vi;
        VkPipelineInputAssemblyStateCreateInfo ia;
        VkPipelineRasterizationStateCreateInfo rs;
        VkPipelineColorBlendStateCreateInfo cb;
        VkPipelineDepthStencilStateCreateInfo ds;
        VkPipelineViewportStateCreateInfo vp;
        VkPipelineMultisampleStateCreateInfo ms;
        VkDynamicState dynamicStateEnables[NUM_DYNAMIC_STATES];
        VkPipelineDynamicStateCreateInfo dynamicState;
        memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
        memset(&dynamicState, 0, sizeof dynamicState);
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = dynamicStateEnables;
        memset(&pipeline_ci, 0, sizeof(pipeline_ci));
        pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_ci.layout = layout;
        memset(&vi, 0, sizeof(vi));
        vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vi.pNext = nullptr;
        vi.vertexBindingDescriptionCount = static_cast<uint32_t>(config.bindings.size());
        vi.pVertexBindingDescriptions = config.bindings.data();
        vi.vertexAttributeDescriptionCount = static_cast<uint32_t>(config.attribs.size());
        vi.pVertexAttributeDescriptions = config.attribs.data();
        memset(&ia, 0, sizeof(ia));
        ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        memset(&rs, 0, sizeof(rs));
        rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        rs.cullMode = VK_CULL_MODE_BACK_BIT;
        rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rs.depthClampEnable = VK_FALSE;
        rs.rasterizerDiscardEnable = VK_FALSE;
        rs.depthBiasEnable = VK_FALSE;
        rs.lineWidth = 1.0f;
        memset(&cb, 0, sizeof(cb));
        cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        VkPipelineColorBlendAttachmentState att_state[1];
        memset(att_state, 0, sizeof(att_state));
        att_state[0].colorWriteMask = 0xf;
        att_state[0].blendEnable = VK_FALSE;
        cb.attachmentCount = 1;
        cb.pAttachments = att_state;
        memset(&vp, 0, sizeof(vp));
        vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        vp.viewportCount = 1;
        dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
        vp.scissorCount = 1;
        dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
        memset(&ds, 0, sizeof(ds));
        ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ds.depthTestEnable = VK_TRUE;
        ds.depthWriteEnable = VK_TRUE;
        ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        ds.depthBoundsTestEnable = VK_FALSE;
        ds.back.failOp = VK_STENCIL_OP_KEEP;
        ds.back.passOp = VK_STENCIL_OP_KEEP;
        ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
        ds.stencilTestEnable = VK_FALSE;
        ds.front = ds.back;
        memset(&ms, 0, sizeof(ms));
        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.pSampleMask = nullptr;
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        auto prepare_shader_module = [](VkDevice device, const char *const code_str, EShLanguage stage) -> VkShaderModule {
            const char *shaderStrings[] = {code_str};
            glslang::TShader shader(stage);
            shader.setStrings(shaderStrings, 1);
            auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
            TBuiltInResource resource = DEFAULT_SHADER_RESOURCE_SIZES;
            if (!shader.parse(&resource, 100, false, messages)) {
                std::cerr << shader.getInfoLog() << '\n'
                          << shader.getInfoDebugLog() << std::endl;
            }
            glslang::TProgram program;
            program.addShader(&shader);
            if (!program.link(messages)) {
                std::cerr << shader.getInfoLog() << '\n'
                          << shader.getInfoDebugLog() << std::endl;
            }
            std::vector<uint32_t> spv_binary;
            glslang::GlslangToSpv(*program.getIntermediate(stage), spv_binary);

            VkShaderModule module;
            VkShaderModuleCreateInfo moduleCreateInfo;
            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCreateInfo.pNext = nullptr;
            moduleCreateInfo.flags = 0;
            moduleCreateInfo.codeSize = spv_binary.size() * sizeof(uint32_t);
            moduleCreateInfo.pCode = spv_binary.data();
            vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &module);
            return module;
        };
        glslang::InitializeProcess();
        vert_shader_module = prepare_shader_module(device_handle, config.vert_src, EShLangVertex);
        frag_shader_module = prepare_shader_module(device_handle, config.frag_src, EShLangFragment);
        glslang::FinalizeProcess();
        VkPipelineShaderStageCreateInfo shaderStages[2];
        memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vert_shader_module;
        shaderStages[0].pName = "main";
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = frag_shader_module;
        shaderStages[1].pName = "main";
        memset(&pipelineCache, 0, sizeof(pipelineCache));
        pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkCreatePipelineCache(device_handle, &pipelineCache, nullptr, &cache);
        pipeline_ci.pVertexInputState = &vi;
        pipeline_ci.pInputAssemblyState = &ia;
        pipeline_ci.pRasterizationState = &rs;
        pipeline_ci.pColorBlendState = &cb;
        pipeline_ci.pMultisampleState = &ms;
        pipeline_ci.pViewportState = &vp;
        pipeline_ci.pDepthStencilState = &ds;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = shaderStages;
        pipeline_ci.renderPass = config.render_pass;
        pipeline_ci.pDynamicState = &dynamicState;
        vkCreateGraphicsPipelines(device_handle, cache, 1, &pipeline_ci, nullptr, &handle);
        vkDestroyShaderModule(device_handle, frag_shader_module, nullptr);
        vkDestroyShaderModule(device_handle, vert_shader_module, nullptr);
    }

    GraphicsPipeline::~GraphicsPipeline() {
        vkDestroyPipeline(device_handle, handle, nullptr);
        vkDestroyPipelineCache(device_handle, cache, nullptr);
        vkDestroyPipelineLayout(device_handle, layout, nullptr);
    }

    void GraphicsPipeline::bind(VkCommandBuffer cmd) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, handle);
    }
} // namespace coel::vulkan
