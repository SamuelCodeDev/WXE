#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "All.h"

#ifdef _WIN32
	#include <D3DCompiler.h>
	#include <DirectXMath.h>
	#include <DirectXColors.h>
	using namespace DirectX;
	using Position = XMFLOAT3;
	using Color = XMFLOAT4;
#endif

struct Vertex
{
	Position Pos;
	Color Color;
};

namespace WXE
{
	class Triangle : public Game
	{
	private:
		ID3D12RootSignature* rootSignature;
		ID3D12PipelineState* pipelineState;
		Mesh* geometry;

	public:
		void Init();
		void Update() noexcept;
		void Display() noexcept;
		void Finalize() noexcept;

		void BuildGeometry() noexcept;
		void BuildRootSignature();
		void BuildPipelineState();
	};
}

#endif