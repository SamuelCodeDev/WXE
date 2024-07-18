#include "Graphics.h"
#include "Error.h"
#include "Utils.h"
#include <format>
using std::format;

namespace WXE::DX12
{
	Graphics::Graphics() noexcept :
        factory { nullptr },
        device { nullptr },
        swapChain { nullptr },
        commandQueue { nullptr },
        commandList { nullptr },
        commandListAlloc { nullptr },
        depthStencil { nullptr },
        renderTargetHeap { nullptr },
        depthStencilHeap { nullptr },
        fence { nullptr },
        currentFence{},
        rtDescriptorSize{}
	{
        backBufferCount = 2;
        backBufferIndex = 0;
        antialiasing = 1;
        quality = 0;
        vSync = false;

        renderTargets = new ID3D12Resource * [backBufferCount] { nullptr };
        
        ZeroMemory(bgColor, sizeof(bgColor));
        ZeroMemory(&viewport, sizeof(viewport));
        ZeroMemory(&scissorRect, sizeof(scissorRect));
	}

    Graphics::~Graphics()
    {
        WaitCommandQueue();

        if (renderTargets)
        {
            for (uint32 i = 0; i < backBufferCount; ++i)
                SafeRelease(renderTargets[i]);
            delete[] renderTargets;
        }

        if (swapChain)
        {
            swapChain->SetFullscreenState(false, nullptr);
            swapChain->Release();
        }

        SafeRelease(depthStencil);
        SafeRelease(fence);
        SafeRelease(depthStencilHeap);
        SafeRelease(renderTargetHeap);
        SafeRelease(commandList);
        SafeRelease(commandListAlloc);
        SafeRelease(commandQueue);
        SafeRelease(device);
        SafeRelease(factory);
    }

    void Graphics::LogHardwareInfo()
    {
        constexpr auto BytesinMegaByte = 1048576U; // 1'048'576

        // --------------------------------------
        // Adapter card (graphics card)
        // --------------------------------------
        IDXGIAdapter* adapter = nullptr;
        if (factory->EnumAdapters(0, &adapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);
            OutputDebugStringW(format(L"---> Graphics card: {}\n", desc.Description).c_str());
        }

        IDXGIAdapter4* adapter4 = nullptr;
        if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter4))))
        {
            DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
            adapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);

            string text = format("---> Video Memory (Free): {} MB\n", memInfo.Budget / BytesinMegaByte);
            text += format("---> Video Memory (Used): {} MB\n", memInfo.CurrentUsage / BytesinMegaByte);

            OutputDebugString(text.c_str());

            adapter4->Release();
        }

        // ------------------------------------------
        // Maximum Level Feature Supported by the GPU
        // ------------------------------------------
        constexpr D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelsInfo {
            .NumFeatureLevels = static_cast<uint32>(countof(featureLevels)),
            .pFeatureLevelsRequested = featureLevels,
        };

        device->CheckFeatureSupport(
            D3D12_FEATURE_FEATURE_LEVELS,
            &featureLevelsInfo,
            sizeof(featureLevelsInfo));
        {
            string text = format("---> Feature Level: ");

            switch (featureLevelsInfo.MaxSupportedFeatureLevel)
            {
            case D3D_FEATURE_LEVEL_12_1: text += "12_1\n"; break;
            case D3D_FEATURE_LEVEL_12_0: text += "12_0\n"; break;
            case D3D_FEATURE_LEVEL_11_1: text += "11_1\n"; break;
            case D3D_FEATURE_LEVEL_11_0: text += "11_0\n"; break;
            case D3D_FEATURE_LEVEL_10_1: text += "10_1\n"; break;
            case D3D_FEATURE_LEVEL_10_0: text += "10_0\n"; break;
            case D3D_FEATURE_LEVEL_9_3:  text += "9_3\n";  break;
            case D3D_FEATURE_LEVEL_9_2:  text += "9_2\n";  break;
            case D3D_FEATURE_LEVEL_9_1:  text += "9_1\n";  break;
            }

            OutputDebugString(text.c_str());
        }

        // -----------------------------------------
        // Video output (monitor)
        // -----------------------------------------

        IDXGIOutput* output = nullptr;
        if (adapter->EnumOutputs(0, &output) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);
            OutputDebugStringW(format(L"---> Monitor: {}\n", desc.DeviceName).c_str());
        }

        // ------------------------------------------
        // Video Mode (resolution)
        // ------------------------------------------

        uint32 dpi = GetDpiForSystem();
        uint32 screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
        uint32 screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);

        DEVMODE devMode { .dmSize = sizeof(DEVMODE) };
        EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
        uint32 refresh = devMode.dmDisplayFrequency;

        OutputDebugString(
            format("--->Resolution: {} x {} {} Hz\n", 
                screenWidth, screenHeight, refresh).c_str());

        SafeRelease(adapter);
        SafeRelease(output);
    }

    void Graphics::Initialize(Window* window)
    {
        // ---------------------------------------------------
        // DXGI infrastructure and D3D device
        // ---------------------------------------------------

        uint32 factoryFlags{};

    #ifdef _DEBUG
        factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
        ID3D12Debug* debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    #endif

        ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)));

        if FAILED(D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device)))
        {
            IDXGIAdapter* warp;
            ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)));

            ThrowIfFailed(D3D12CreateDevice(
                warp,
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&device)));

            warp->Release();

            OutputDebugString("---> Using Warp Adapter: There is no support to D3D12\n");
        }

    #ifdef _DEBUG
        LogHardwareInfo();
    #endif 

        // ---------------------------------------------------
        // Queue, List and Command Allocator
        // ---------------------------------------------------

        D3D12_COMMAND_QUEUE_DESC queueDesc {
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        };
        ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, 
            IID_PPV_ARGS(&commandListAlloc)));

        ThrowIfFailed(device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            commandListAlloc,
            nullptr,
            IID_PPV_ARGS(&commandList)));

        // ---------------------------------------------------
        // CPU/GPU synchronization fence
        // ---------------------------------------------------

        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        // ---------------------------------------------------
        // Swap Chain
        // ---------------------------------------------------

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
            .Width = static_cast<uint32>(window->Width()),
            .Height = static_cast<uint32>(window->Height()),
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc {
                .Count = antialiasing,
                .Quality = quality,
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = backBufferCount,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
        };

        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            commandQueue,
            window->Id(),
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain));

        // ---------------------------------------------------
        // Render Target Views (and associated heaps)
        // ---------------------------------------------------

        D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = backBufferCount,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        };
        ThrowIfFailed(device->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&renderTargetHeap)));

        D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = renderTargetHeap->GetCPUDescriptorHandleForHeapStart();

        rtDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (uint32 i = 0; i < backBufferCount; ++i)
        {
            swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
            device->CreateRenderTargetView(renderTargets[i], nullptr, rtHandle);
            rtHandle.ptr += rtDescriptorSize;
        }

        // ---------------------------------------------------
        // Depth/Stencil View (and associated heaps)
        // ---------------------------------------------------

        D3D12_RESOURCE_DESC depthStencilDesc {
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0,
            .Width = static_cast<uint64>(window->Width()),
            .Height = static_cast<uint32>(window->Height()),
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc {
                .Count = antialiasing,
                .Quality = quality,
            },
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
        };

        D3D12_HEAP_PROPERTIES dsHeapProperties {
            .Type = D3D12_HEAP_TYPE_DEFAULT,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 1,
            .VisibleNodeMask = 1,
        };

        D3D12_CLEAR_VALUE optmizedClear {
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .DepthStencil{
                .Depth = 1.0f,
                .Stencil = 0,
            },
        };

        ThrowIfFailed(device->CreateCommittedResource(
            &dsHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &optmizedClear,
            IID_PPV_ARGS(&depthStencil)));

        D3D12_DESCRIPTOR_HEAP_DESC depthstencilHeapDesc {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        };
        ThrowIfFailed(device->CreateDescriptorHeap(&depthstencilHeapDesc, IID_PPV_ARGS(&depthStencilHeap)));

        D3D12_CPU_DESCRIPTOR_HANDLE dsHandle = depthStencilHeap->GetCPUDescriptorHandleForHeapStart();

        device->CreateDepthStencilView(depthStencil, nullptr, dsHandle);

        D3D12_RESOURCE_BARRIER barrier {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition {
                .pResource = depthStencil,
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_COMMON,
                .StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE,
            },
        };
        commandList->ResourceBarrier(1, &barrier);

        SubmitCommands();

        // ---------------------------------------------------
        // Viewport and Scissor Rect
        // ---------------------------------------------------

        viewport.TopLeftX = {};
        viewport.TopLeftY = {};
        viewport.Width = static_cast<float>(window->Width());
        viewport.Height = static_cast<float>(window->Height());
        viewport.MinDepth = {};
        viewport.MaxDepth = 1.0f;

        scissorRect = { 0, 0, window->Width(), window->Height() };

        COLORREF color { window->Color() };

        bgColor[0] = GetRValue(color) / 255.0f;
        bgColor[1] = GetGValue(color) / 255.0f;
        bgColor[2] = GetBValue(color) / 255.0f;
        bgColor[3] = 1.0f;
    }

    void Graphics::Clear(ID3D12PipelineState* pso)
    {
        commandListAlloc->Reset();

        commandList->Reset(commandListAlloc, pso);

        D3D12_RESOURCE_BARRIER barrier {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition {
                .pResource = renderTargets[backBufferIndex],
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
            },
        };
        commandList->ResourceBarrier(1, &barrier);

        commandList->RSSetViewports(1, reinterpret_cast<D3D12_VIEWPORT*>(&viewport));
        commandList->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&scissorRect));

        D3D12_CPU_DESCRIPTOR_HANDLE dsHandle = depthStencilHeap->GetCPUDescriptorHandleForHeapStart();
        D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = renderTargetHeap->GetCPUDescriptorHandleForHeapStart();
        rtHandle.ptr += SIZE_T(backBufferIndex) * SIZE_T(rtDescriptorSize);
        commandList->ClearRenderTargetView(rtHandle, bgColor, 0, nullptr);
        commandList->ClearDepthStencilView(dsHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        commandList->OMSetRenderTargets(1, &rtHandle, true, &dsHandle);
    }

    bool Graphics::WaitCommandQueue() noexcept
    {
        currentFence++;

        if (FAILED(commandQueue->Signal(fence, currentFence)))
            return false;

        if (fence->GetCompletedValue() < currentFence)
        {
            HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

            if (eventHandle)
            {
                if (FAILED(fence->SetEventOnCompletion(currentFence, eventHandle)))
                    return false;

                WaitForSingleObject(eventHandle, INFINITE);
                CloseHandle(eventHandle);
            }
        }

        return true;
    }

    void Graphics::SubmitCommands() noexcept
    {
        commandList->Close();
        ID3D12CommandList* cmdsLists[] { commandList };
        commandQueue->ExecuteCommandLists(static_cast<uint32>(countof(cmdsLists)), cmdsLists);
        WaitCommandQueue();
    }

    void Graphics::Allocate(const uint32 type, 
                            const uint32 sizeInBytes, 
                            ID3D12Resource** resource) const
    {
        D3D12_HEAP_PROPERTIES bufferProp {
            .Type = (type == UPLOAD) ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 1,
            .VisibleNodeMask = 1,
        };

        D3D12_RESOURCE_DESC bufferDesc {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = sizeInBytes,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc {
                .Count = 1,
                .Quality = 0,
            },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE,
        };
        
        D3D12_RESOURCE_STATES initState { 
            (type == UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON
        };

        ThrowIfFailed(device->CreateCommittedResource(
            &bufferProp,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            initState,
            nullptr,
            IID_PPV_ARGS(resource)));
    }

    void Graphics::Copy(const void* vertices, 
                        const uint32 sizeInBytes,
                        ID3D12Resource* bufferUpload, 
                        ID3D12Resource* bufferGPU) noexcept
    {
        // ---------------------------------------------------
        // Copy vertices for the standard buffer (GPU)
        // ---------------------------------------------------

        D3D12_SUBRESOURCE_DATA vertexSubResourceData {
            .pData = vertices,
            .RowPitch = sizeInBytes,
            .SlicePitch = sizeInBytes,
        };

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts{};
        uint32 numRows{};
        uint64 rowSizesInBytes{};
        uint64 requiredSize{};

        D3D12_RESOURCE_DESC bufferGPUDesc = bufferGPU->GetDesc();

        device->GetCopyableFootprints(
            &bufferGPUDesc,
            0, 1, 0, &layouts, &numRows,
            &rowSizesInBytes, &requiredSize);

        BYTE* pData{};
        bufferUpload->Map(0, nullptr, reinterpret_cast<void**>(&pData));

        D3D12_MEMCPY_DEST DestData =
        {
            pData + layouts.Offset,
            layouts.Footprint.RowPitch,
            layouts.Footprint.RowPitch * uint64(numRows)
        };

        for (uint32 z = 0; z < layouts.Footprint.Depth; ++z)
        {
            BYTE* destSlice = reinterpret_cast<BYTE*>(DestData.pData) + DestData.SlicePitch * z;

            const BYTE* srcSlice = reinterpret_cast<const BYTE*>(vertexSubResourceData.pData) + vertexSubResourceData.SlicePitch * z;

            for (uint32 y = 0; y < numRows; ++y)
                memcpy(destSlice + DestData.RowPitch * y,
                    srcSlice + vertexSubResourceData.RowPitch * y,
                    static_cast<size_t>(rowSizesInBytes));
        }

        bufferUpload->Unmap(0, nullptr);

        D3D12_RESOURCE_BARRIER barrier {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition {
                .pResource = bufferGPU,
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_COMMON,
                .StateAfter = D3D12_RESOURCE_STATE_COPY_DEST,
            },
        };
        commandList->ResourceBarrier(1, &barrier);

        commandList->CopyBufferRegion(
            bufferGPU,
            0,
            bufferUpload,
            layouts.Offset,
            layouts.Footprint.Width);

        barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition {
                .pResource = bufferGPU,
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_COPY_DEST,
                .StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ,
            }
        };
        commandList->ResourceBarrier(1, &barrier);
    }

    void Graphics::Present() noexcept
    {
        D3D12_RESOURCE_BARRIER barrier {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition {
                .pResource = renderTargets[backBufferIndex],
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
                .StateAfter = D3D12_RESOURCE_STATE_PRESENT,
            },
        };

        commandList->ResourceBarrier(1, &barrier);

        SubmitCommands();

        swapChain->Present(vSync, 0);
        backBufferIndex = (backBufferIndex + 1) % backBufferCount;
    }
}