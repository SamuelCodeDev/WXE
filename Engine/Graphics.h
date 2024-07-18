#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Types.h"

#ifdef _WIN32
	#include "Window.h"
	#include <D3DCompiler.h>
	#include <dxgi1_6.h>
	#include <d3d12.h>
	
	#pragma comment(lib, "d3dcompiler.lib")
	#pragma comment(lib, "dxgi.lib")
	#pragma comment(lib, "d3d12.lib")
	using Window = WXE::Windows::Window;

	enum AllocationType { GPU, UPLOAD };
#endif

namespace WXE
{
	class GraphicsDesc
	{
	protected:
		uint32   backBufferCount;
		uint32   antialiasing;
		uint32   quality;
		bool     vSync;
		float    bgColor[4];
		Rect     scissorRect;
		ViewPort viewport;

	public:
		uint32 Antialiasing() const noexcept;
		uint32 Quality() const noexcept;
		void VSync(const bool state) noexcept;
	};

	inline uint32 GraphicsDesc::Antialiasing() const noexcept
	{ return antialiasing; }

	inline uint32 GraphicsDesc::Quality() const noexcept
	{ return quality; }

	inline void GraphicsDesc::VSync(const bool state) noexcept
	{ vSync = state; }
}

namespace WXE::DX12
{
	class Graphics : public GraphicsDesc
	{
	private:
		ID3D12Device4* device;
		IDXGIFactory6* factory;
		IDXGISwapChain1* swapChain;
		uint8					    backBufferIndex;

		ID3D12CommandQueue* commandQueue;
		ID3D12GraphicsCommandList* commandList;
		ID3D12CommandAllocator* commandListAlloc;

		ID3D12Resource** renderTargets;
		ID3D12Resource* depthStencil;
		ID3D12DescriptorHeap* renderTargetHeap;
		ID3D12DescriptorHeap* depthStencilHeap;
		uint32					    rtDescriptorSize;

		ID3D12Fence* fence;
		uint64					    currentFence;

		void LogHardwareInfo();
		bool WaitCommandQueue() noexcept;

	public:
		Graphics() noexcept;
		~Graphics();

		void Initialize(Window* window);
		void Clear(ID3D12PipelineState* pso);
		void Present() noexcept;

		void ResetCommands() const noexcept;
		void SubmitCommands() noexcept;

		void Allocate(const uint32 sizeInBytes,
			ID3DBlob** resource) const noexcept;

		void Allocate(const uint32 type,
			const uint32 sizeInBytes,
			ID3D12Resource** resource) const;

		void Copy(const void* vertices,
			const uint32 sizeInBytes,
			ID3DBlob* bufferCPU) const noexcept;

		void Copy(const void* vertices,
				  const uint32 sizeInBytes,
				  ID3D12Resource* bufferUpload,
				  ID3D12Resource* bufferGPU) noexcept;

		ID3D12Device4* Device() const noexcept;
		ID3D12GraphicsCommandList* CommandList() const noexcept;
	};

	inline ID3D12Device4* Graphics::Device() const noexcept
	{ return device; }

	inline ID3D12GraphicsCommandList* Graphics::CommandList() const noexcept
	{ return commandList; }

	inline void Graphics::ResetCommands() const noexcept
	{ commandList->Reset(commandListAlloc, nullptr); }

	void Graphics::Allocate(const uint32 sizeInBytes, ID3DBlob ** resource) const noexcept
	{ D3DCreateBlob(sizeInBytes, resource); }

	void Graphics::Copy(const void* vertices, const uint32 sizeInBytes, ID3DBlob * bufferCPU) const noexcept
	{ CopyMemory(bufferCPU->GetBufferPointer(), vertices, sizeInBytes); }
}

#endif