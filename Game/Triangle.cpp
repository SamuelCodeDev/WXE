#include "Triangle.h"

namespace WXE
{
    void Triangle::Init()
    {
        graphics->ResetCommands();
    
        BuildGeometry();
        BuildRootSignature();
        BuildPipelineState();
        
        graphics->SubmitCommands();
    }

    void Triangle::Update() noexcept
    {
        if (input->KeyPress(VK_ESCAPE))
            window->Close();
    }
    
    void Triangle::Display() noexcept
    {
        graphics->Clear(pipelineState);

        graphics->CommandList()->SetGraphicsRootSignature(rootSignature);
        graphics->CommandList()->IASetVertexBuffers(0, 1, geometry->VertexBufferView());
        graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        graphics->CommandList()->DrawInstanced(3, 1, 0, 0);

        graphics->Present();
    }

    void Triangle::Finalize() noexcept
    {
        rootSignature->Release();
        pipelineState->Release();
        delete geometry;
    }

    void Triangle::BuildGeometry() noexcept
    {
        Vertex vertices[] =
        {
            { Position(0.0f, 0.5f, 0.0f), Color(Colors::Red) },
            { Position(0.5f, -0.5f, 0.0f), Color(Colors::Orange) },
            { Position(-0.5f, -0.5f, 0.0f), Color(Colors::Yellow) }
        };

        constexpr auto vbSize { countof(vertices) * sizeof(Vertex) };

        geometry = new Mesh("Triangle");

        geometry->vertexByteStride = sizeof(Vertex);
        geometry->vertexBufferSize = vbSize;

        graphics->Allocate(vbSize, &geometry->vertexBufferCPU);
        graphics->Allocate(UPLOAD, vbSize, &geometry->vertexBufferUpload);
        graphics->Allocate(GPU, vbSize, &geometry->vertexBufferGPU);

        graphics->Copy(vertices, vbSize, geometry->vertexBufferCPU);
        graphics->Copy(vertices, vbSize, geometry->vertexBufferUpload, geometry->vertexBufferGPU);
    }

    void Triangle::BuildRootSignature()
    {
        D3D12_ROOT_SIGNATURE_DESC rootSigDesc {
            .NumParameters = 0,
            .pParameters = nullptr,
            .NumStaticSamplers = 0,
            .pStaticSamplers = nullptr,
            .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
        };
        
        ID3DBlob* serializedRootSig{ nullptr };
        ID3DBlob* error{ nullptr };

        ThrowIfFailed(D3D12SerializeRootSignature(
            &rootSigDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig,
            &error));

        ThrowIfFailed(graphics->Device()->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)));
    }

    void Triangle::BuildPipelineState()
    {
        // --------------------
        // --- Input Layout ---
        // --------------------

        D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // --------------------
        // ----- Shaders ------
        // --------------------

        ID3DBlob* vertexShader;
        ID3DBlob* pixelShader;

        D3DReadFileToBlob(L"../Engine/Shaders/Vertex.cso", &vertexShader);
        D3DReadFileToBlob(L"../Engine/Shaders/Pixel.cso", &pixelShader);

        // --------------------
        // ---- Rasterizer ----
        // --------------------

        D3D12_RASTERIZER_DESC rasterizer {
            .FillMode = D3D12_FILL_MODE_SOLID,
            //.FillMode = D3D12_FILL_MODE_WIREFRAME,
            .CullMode = D3D12_CULL_MODE_BACK,
            .FrontCounterClockwise = FALSE,
            .DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
            .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
            .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
            .DepthClipEnable = TRUE,
            .MultisampleEnable = FALSE,
            .AntialiasedLineEnable = FALSE,
            .ForcedSampleCount = 0,
            .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
        };

        // ---------------------
        // --- Color Blender ---
        // ---------------------

        D3D12_BLEND_DESC blender {
            .AlphaToCoverageEnable = FALSE,
            .IndependentBlendEnable = FALSE,
        };

        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for (uint32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            blender.RenderTarget[i] = defaultRenderTargetBlendDesc;

        // ---------------------
        // --- Depth Stencil ---
        // ---------------------

        constexpr D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };

        D3D12_DEPTH_STENCIL_DESC depthStencil {
            .DepthEnable = TRUE,
            .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
            .StencilEnable = FALSE,
            .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
            .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
            .FrontFace = defaultStencilOp,
            .BackFace = defaultStencilOp,
        };

        // -----------------------------------
        // --- Pipeline State Object (PSO) ---
        // -----------------------------------

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso {
            .pRootSignature = rootSignature,
            .VS = { reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() },
            .PS = { reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() },
            .BlendState = blender,
            .SampleMask = UINT_MAX,
            .RasterizerState = rasterizer,
            .DepthStencilState = depthStencil,
            .InputLayout = { inputLayout, 2 },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
            .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc{
                .Count = graphics->Antialiasing(),
                .Quality = graphics->Quality(),
            },
        };
        graphics->Device()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipelineState));

        vertexShader->Release();
        pixelShader->Release();
    }
}

#ifdef _WIN32
auto APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow) -> int
{
    using namespace WXE;

    try
    {
        Engine* engine = new Engine();
        engine->window->Mode(WINDOWED);
        engine->window->Size(600, 600);
        engine->window->Color(0, 122, 204);
        engine->window->Title("Triangle");
        engine->window->Icon(IDI_ICON);
        engine->window->Cursor(IDC_CURSOR);
        engine->window->LostFocus(Engine::Pause);
        engine->window->InFocus(Engine::Resume);

        int exit = engine->Start(new Triangle());

        delete engine;
        return exit;
    }
    catch (Error& e)
    {
        MessageBox(nullptr, e.ToString().data(), "Triangle", MB_OK);
        return 0;
    }
}
#else
int main(int argc, char ** argv)
{

}
#endif