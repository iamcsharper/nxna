#include "GlRenderTarget2D.h"
#include "GlTexture2D.h"
#include "OpenGL.h"
#include "../GraphicsDevice.h"

namespace Nxna
{
namespace Graphics
{
namespace OpenGl
{
	GlRenderTarget2D::GlRenderTarget2D(OpenGlDevice* device, GlTexture2D* texture, int width, int height, SurfaceFormat preferredFormat, DepthFormat preferredDepthFormat, int preferredMultiSampleCount, RenderTargetUsage usage)
	{
		m_stencilBuffer = 0;
		
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		// attach the texture to the FBO
		glBindTexture(GL_TEXTURE_2D, texture->GetGlTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef USING_OPENGLES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
#endif
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->GetGlTexture(), 0);

		// generate the depth buffer (if needed)
		if (preferredDepthFormat != DepthFormat::None)
		{
			glGenRenderbuffers(1, &m_depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);

			if (preferredDepthFormat == DepthFormat::Depth16)
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
			else if (preferredDepthFormat == DepthFormat::Depth24)
#ifdef USING_OPENGLES
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
#else
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
#endif
			else if (preferredDepthFormat == DepthFormat::Depth24Stencil8)
#ifdef USING_OPENGLES
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, width, height);
#else
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
#endif

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);

			if (preferredDepthFormat == DepthFormat::Depth24Stencil8)
			{
#ifdef USING_OPENGL
				glGenRenderbuffers(1, &m_stencilBuffer);
				glBindRenderbuffer(GL_RENDERBUFFER, m_stencilBuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
#else
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
#endif
			}
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			throw GraphicsException("Unable to create render target");
	}
}
}
}