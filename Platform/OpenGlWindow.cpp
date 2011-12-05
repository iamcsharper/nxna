#include "PlatformDefs.h"
#include "OpenGlWindow.h"
#include "../Graphics/OpenGL/OpenGLDevice.h"

#ifdef NXNA_PLATFORM_NACL
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_3d_client.h>
#include <ppapi/cpp/graphics_3d.h>

extern pp::Instance* g_Instance;
#else
#include <SDL.h>
#endif

namespace Nxna
{
namespace Platform
{
#ifdef NXNA_PLATFORM_NACL
	class OpenGlContext : public pp::Graphics3DClient
	{
		explicit OpenGlContext(pp::Instance* instance, int width, int height)
			: pp::Graphics3DClient(instance)
		{
			int attribs[] = 
			{
				PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
				PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
				PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
				PP_GRAPHICS3DATTRIB_SAMPLES, 0,
				PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
				PP_GRAPHICS3DATTRIB_WIDTH, width,
				PP_GRAPHICS3DATTRIB_HEIGHT, height,
				PP_GRAPHICS3DATTRIB_NONE
			};

			context_ = pp::Graphics3D(*intance, pp::Graphics3D, attribs);

			instance->BindGraphics(context_);
		}

		virtual ~OpenGlContext()
		{
			glSetCurrentContextPPAPI(0);
		}
	};

#endif


	OpenGlWindow::OpenGlWindow(Nxna::Game* game)
		: Nxna::GraphicsDeviceManager(game)
	{
		
	}

	Nxna::Graphics::GraphicsDevice* OpenGlWindow::CreateDevice()
	{
		m_device = new Nxna::Graphics::OpenGl::OpenGlDevice();

		return m_device;
	}

	void OpenGlWindow::BeginDraw()
	{
	}

	void OpenGlWindow::EndDraw()
	{
#ifndef NXNA_PLATFORM_NACL
		SDL_GL_SwapWindow((SDL_Window*)m_window);
#endif
	}

	void OpenGlWindow::ApplyChanges()
	{
	}

	void OpenGlWindow::ShowWindow()
	{
	}

	void OpenGlWindow::DestroyWindow()
	{
#ifndef NXNA_PLATFORM_NACL
		SDL_DestroyWindow((SDL_Window*)m_window);
		SDL_Quit();
#endif
	}

	void OpenGlWindow::SetScreenSize(int width, int height, bool fullscreen)
	{
#ifndef NXNA_PLATFORM_NACL
        PreferredBackBufferWidth(width);
		PreferredBackBufferHeight(height);

		int bitsPerPixel = 32;

		SDL_Init(SDL_INIT_VIDEO);
        SDL_SetHint( "SDL_HINT_ORIENTATIONS", "LandscapeLeft LandscapeRight" );

		if (bitsPerPixel == 16)
		{
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16);
		}
		else
		{
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);
		}
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        
        int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
#ifdef EMBEDDED
        flags |= SDL_WINDOW_BORDERLESS;
#endif
        
		m_window = SDL_CreateWindow("CNK", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			PreferredBackBufferWidth(), PreferredBackBufferHeight(), flags);
        
        SDL_GetWindowSize((SDL_Window*)m_window, &width, &height);
        
        PreferredBackBufferWidth(width);
        PreferredBackBufferHeight(height);
        
		SDL_GLContext context = SDL_GL_CreateContext((SDL_Window*)m_window);

		static_cast<Nxna::Graphics::OpenGl::OpenGlDevice*>(m_device)->OnContextCreated();
		static_cast<Nxna::Graphics::OpenGl::OpenGlDevice*>(m_device)->UpdateScreenSize(PreferredBackBufferWidth(), PreferredBackBufferHeight());

		m_device->SetViewport(Nxna::Graphics::Viewport(0, 0, PreferredBackBufferWidth(), PreferredBackBufferHeight()));
#else
		OpenGlContext* context = new OpenGlContext(g_Instance);
#endif
	}
}
}