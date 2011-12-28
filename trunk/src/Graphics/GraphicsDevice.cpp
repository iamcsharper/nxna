#include "GraphicsDevice.h"
#include "GraphicsDeviceCapabilities.h"
#include "../Matrix.h"
#include "../Vector4.h"

namespace Nxna
{
namespace Graphics
{
	Nxna::Vector3 Viewport::Unproject(const Nxna::Vector3& source, 
			const Nxna::Matrix& projection, 
			const Nxna::Matrix& view, 
			const Nxna::Matrix& world)
	{
		Nxna::Vector4 result;
        result.X = ((source.X - X) * 2 / Width) - 1;
        result.Y = 1 - ((source.Y - Y) * 2 / Height);
        result.Z = source.Z;
		result.W = 1.0f;

        Nxna::Matrix invProj, invView, invWorld;
        Nxna::Matrix::Invert(projection, invProj);
        Nxna::Matrix::Invert(view, invView);
        Nxna::Matrix::Invert(world, invWorld);

		Nxna::Vector4::Transform(result, invProj, result);
		Nxna::Vector4::Transform(result, invView, result);
		Nxna::Vector4::Transform(result, invWorld, result);

        result = result / result.W;

        return Nxna::Vector3(result.X, result.Y, result.Z);
	}

	GraphicsDevice* GraphicsDevice::m_instance = nullptr;

	GraphicsDevice::~GraphicsDevice()
	{
		delete m_caps;
	}
}
}