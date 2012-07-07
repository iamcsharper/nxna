#include "SDLGame.h"
#include <SDL.h>
#include "../../Audio/AudioManager.h"

namespace Nxna
{
namespace Platform
{
namespace SDL
{
	SDLGame::SDLGame(Game* game)
	{
		m_game = game;
		m_active = true;
		m_quitReceived = false;
		m_targetElapsedTime = 1.0f / 60.0f;
		m_isFixedTimeStep = true;
	}

	void SDLGame::Init()
	{
		if (m_game->m_graphicsDeviceManager != nullptr)
		{
			m_game->m_graphicsDeviceManager->PreferredBackBufferWidth(800);
			m_game->m_graphicsDeviceManager->PreferredBackBufferHeight(480);
			m_game->m_device = m_game->m_graphicsDeviceManager->CreateDevice();
		}

		Audio::AudioManager::Init();

		m_game->Initialize();

		if (m_game->m_graphicsDeviceManager != nullptr)
			m_game->m_graphicsDeviceManager->ShowWindow();
	}

	void SDLGame::Run()
	{
		float timeAtLastUpdate = 0;
		float timeAtLastDraw = 0;

		while(m_quitReceived == false)
		{
			handleEvents();
			updateTime();
		
			bool needToDraw = false;

			if (m_isFixedTimeStep)
			{
				int timesToRun = (int)((m_realTime.TotalGameTime - timeAtLastUpdate) / m_targetElapsedTime);
				if (timesToRun > 20)
					timesToRun = 20;

				for (int i = 0; i < timesToRun; i++)
				{
					m_gameTime.TotalGameTime += m_targetElapsedTime;
					m_gameTime.ElapsedGameTime = m_targetElapsedTime;

					m_game->Update(m_gameTime);

					timeAtLastUpdate = m_realTime.TotalGameTime;
					needToDraw = true;
				}
			}
			else
			{
				float elapsedtime = MathHelper::Min(0.1f, m_gameTime.ElapsedGameTime);

				m_gameTime.TotalGameTime += elapsedtime;
				m_gameTime.ElapsedGameTime = elapsedtime;

				m_game->Update(m_gameTime);

				needToDraw = true;
			}

			if (needToDraw)
			{
				GameTime time;
				time.TotalGameTime = m_gameTime.TotalGameTime;
				time.ElapsedGameTime = m_realTime.TotalGameTime - timeAtLastDraw;

				m_game->m_graphicsDeviceManager->BeginDraw();
				m_game->Draw(time);
				m_game->m_graphicsDeviceManager->EndDraw();

				timeAtLastDraw = m_realTime.TotalGameTime;
			}
		}
	}

	void SDLGame::Exit()
	{
		m_quitReceived = true;
	}

	void SDLGame::IsActive(bool active)
	{
		m_active = active;
		
		if (active)
			m_game->OnActivated();
		else
			m_game->OnDeactivated();
	}

	Nxna::Input::Keys convertSDLK(SDL_Keycode sdlk)
	{
		if (sdlk == SDLK_BACKSPACE)
			return Nxna::Input::Keys_Back;
		if (sdlk == SDLK_RETURN)
			return Nxna::Input::Keys_Enter;
		if (sdlk == SDLK_BACKQUOTE)
			return Nxna::Input::Keys_OemTilde;

		return Nxna::Input::Keys_None;
	}

	void SDLGame::handleEvents()
	{
		Nxna::Input::Touch::TouchPanel::Refresh();

		SDL_Event e;

		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
			case SDL_QUIT:
				m_quitReceived = true;
				break;
			case SDL_MOUSEMOTION:
				Nxna::Input::Mouse::InjectMouseMove(e.motion.x, e.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				Nxna::Input::Mouse::InjectMouseButton(e.button.button, true);
				break;
			case SDL_MOUSEBUTTONUP:
				Nxna::Input::Mouse::InjectMouseButton(e.button.button, false);
				break;
			case SDL_KEYDOWN:
				Nxna::Input::Keyboard::InjectKeyDown(convertSDLK(e.key.keysym.sym));
				break;
			case SDL_KEYUP:
				Nxna::Input::Keyboard::InjectKeyUp(convertSDLK(e.key.keysym.sym));
				break;
			}
		}
	}

	void SDLGame::updateTime()
	{
		unsigned int time = SDL_GetTicks();
		float currentTime = time / 1000.0f;

		m_realTime.ElapsedGameTime = currentTime - m_realTime.TotalGameTime;
		m_realTime.TotalGameTime = currentTime;
	}
}
}
}