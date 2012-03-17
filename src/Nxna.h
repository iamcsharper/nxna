#ifndef NXNA_H
#define NXNA_H

namespace Nxna
{
	enum PlayerIndex
	{
		PlayerIndex_One,
		PlayerIndex_Two,
		PlayerIndex_Three,
		PlayerIndex_Four
	};
}

#include "GameTime.h"
#include "Game.h"
#include "Rectangle.h"
#include "Point.h"
#include "MathHelper.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "IGraphicsDeviceManager.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/VertexDeclaration.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/Texture2D.h"
#include "Graphics/BasicEffect.h"
#include "Graphics/DualTextureEffect.h"
#include "Graphics/AlphaTestEffect.h"
#include "Graphics/SpriteSheet.h"
#include "Graphics/SpriteFont.h"
#include "Graphics/SamplerState.h"
#include "Input/Mouse.h"
#include "Input/Keyboard.h"
#include "Input/GamePad.h"
#include "Input/Buttons.h"
#include "Input/Keys.h"
#include "Input/Touch/TouchPanel.h"
#include "Content/ContentManager.h"

#include "Audio/SoundEffect.h"
#include "Audio/AudioEmitter.h"
#include "Audio/AudioListener.h"

#endif // NXNA_H