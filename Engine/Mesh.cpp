#include "Mesh.h"
#include "Utils.h"

namespace WXE 
{
    Mesh::Mesh(const string name) noexcept :
        id{ name },
        vertexBufferCPU{ nullptr },
        vertexBufferGPU{ nullptr },
        vertexBufferUpload{ nullptr },
        vertexByteStride{},
        vertexBufferSize{}
    {
    }

    Mesh::~Mesh() noexcept
    {
        SafeRelease(vertexBufferUpload);
        SafeRelease(vertexBufferGPU);
        SafeRelease(vertexBufferCPU);
    }

    D3D12_VERTEX_BUFFER_VIEW* Mesh::VertexBufferView() noexcept
    {
        vertexBufferView = {
            .BufferLocation = vertexBufferGPU->GetGPUVirtualAddress(),
            .SizeInBytes = vertexBufferSize,
            .StrideInBytes = vertexByteStride,
        };

        return &vertexBufferView;
    }
}