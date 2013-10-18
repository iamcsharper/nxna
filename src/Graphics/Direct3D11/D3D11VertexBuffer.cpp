#include "../../NxnaConfig.h"

#if !defined NXNA_DISABLE_D3D11

#include <cassert>
#include "Direct3D11Device.h"
#include "D3D11VertexBuffer.h"
#include "../VertexDeclaration.h"
#include "d3d11.h"

namespace Nxna
{
namespace Graphics
{
namespace Direct3D11
{
	D3D11VertexBuffer::D3D11VertexBuffer(bool dynamic, Direct3D11Device* device, const VertexDeclaration* vertexDeclaration, int vertexCount, BufferUsage usage)
		: m_declaration(*vertexDeclaration)
	{
		m_dynamic = dynamic;
		m_device = device;
		m_vertexCount = vertexCount;

		m_buffer = nullptr;
	}

	void D3D11VertexBuffer::SetData(int offsetInBytes, void* data, int vertexCount)
	{
		if (m_dynamic)
			setDataDynamic(offsetInBytes, data, vertexCount);
		else
			setDataStatic(offsetInBytes, data, vertexCount);
	}

	void D3D11VertexBuffer::setDataStatic(int offsetInBytes, void* data, int numVertices)
	{
		// TODO
		assert(offsetInBytes == 0);

		//assert(vertexCount == m_vertexCount);

		int numBytes = m_declaration.GetStride() * m_vertexCount;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = m_declaration.GetStride() * m_vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = data;
		if (FAILED(static_cast<ID3D11Device*>(m_device->GetDevice())->CreateBuffer(&desc, &initData, (ID3D11Buffer**)&m_buffer)))
			throw GraphicsException("Unable to create vertex buffer");
	}

	void D3D11VertexBuffer::setDataDynamic(int offsetInBytes, void* data, int numVertices)
	{
		// TODO
		assert(offsetInBytes == 0);


		if (numVertices > m_vertexCount)
			throw GraphicsException("Too many vertices", __FILE__, __LINE__);

		if (numVertices > 0 && m_buffer == nullptr)
		{
			int numBytes = m_declaration.GetStride() * m_vertexCount;

			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.ByteWidth = numBytes;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			if (FAILED(static_cast<ID3D11Device*>(m_device->GetDevice())->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_buffer)))
				throw GraphicsException("Unable to create dynamic vertex buffer");
		}

		int numBytes = m_declaration.GetStride() * numVertices;

		D3D11_MAPPED_SUBRESOURCE mappedBuffer;

		ID3D11DeviceContext* deviceContext = static_cast<ID3D11DeviceContext*>(m_device->GetDeviceContext());

		if (FAILED(deviceContext->Map(static_cast<ID3D11Buffer*>(m_buffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer)))
			throw GraphicsException("Unable to create dynamic vertex buffer");

		memcpy(mappedBuffer.pData, data, numBytes);

		deviceContext->Unmap(static_cast<ID3D11Buffer*>(m_buffer), 0);
	}
}
}
}

#endif