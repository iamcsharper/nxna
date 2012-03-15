#include <cassert>
#include <cstring>
#include "Texture2D.h"
#include "GraphicsDevice.h"
#include "../Content/FileStream.h"
#include "../Content/ContentManager.h"
#include "../Content/XnbReader.h"

namespace Nxna
{
namespace Graphics
{
	void* Texture2DLoader::Load(Content::XnbReader* stream)
	{
		return Texture2D::LoadFrom(stream);
	}

	void Texture2DLoader::Destroy(void* resource)
	{
		delete static_cast<Texture2D*>(resource);
	}

	Texture2D* Texture2D::LoadFrom(Content::XnbReader* stream)
	{
		int typeID = stream->ReadTypeID();
		
		return Texture2D::LoadFrom(stream->GetStream());
	}

	Texture2D* Texture2D::LoadFrom(Content::FileStream* stream)
	{
		assert(stream != nullptr);
        
        const int FormatColor = 0;
		const int FormatBGR565 = 1;
		const int FormatDXT1 = 4;
		const int FormatDXT3 = 5;
		const int FormatDXT5 = 6;
		const int FormatPVRTC4 = 100; // this is not supported by XNA! this is own own extension!

		int format = stream->ReadInt32();
		if (format != FormatColor && format != FormatBGR565 && format != FormatDXT1 && format != FormatDXT3 && format != FormatPVRTC4)
			throw Content::ContentException("Unsupported texture format");

		int width = stream->ReadInt32();
		int height = stream->ReadInt32();
		int mipCount = stream->ReadInt32();
        
        Texture2D* texture;
		if (format == FormatDXT1)
			texture = GraphicsDevice::GetDevice()->CreateTexture(width, height, SurfaceFormat_Dxt1);
		else if (format == FormatDXT3)
			texture = GraphicsDevice::GetDevice()->CreateTexture(width, height, SurfaceFormat_Dxt3);
		else if (format == FormatPVRTC4)
			texture = GraphicsDevice::GetDevice()->CreateTexture(width, height, SurfaceFormat_Pvrtc4);
		else
			texture = GraphicsDevice::GetDevice()->CreateTexture(width, height, SurfaceFormat_Color);

		byte* pixels = nullptr;
		int imageSize;
		for (int i = 0; i < mipCount; i++)
		{
			int size = stream->ReadInt32();
			
			pixels = new byte[size];

			stream->Read(pixels, size);
			imageSize = size;

			if (format == 1)
			{
				byte* convertedPixels = convert(pixels, size / 2, format);
				delete[] pixels;
				pixels = convertedPixels;
			}

			texture->SetData(i, pixels, imageSize);

			delete[] pixels;
		}

		return texture;
	}

	byte* Texture2D::convert(byte* pixels, int length, int format)
	{
		if (format == 1)
		{
			byte* result = new byte[length * 4];

			for (int i = 0; i < length; i++)
			{
				unsigned short pixel;
				memcpy(&pixel, &pixels[i * 2], 2);

				byte r, g, b;
				convert565(pixel, &r, &g, &b);

				result[i * 4 + 0] = r;
				result[i * 4 + 1] = g;
				result[i * 4 + 2] = b;
				result[i * 4 + 3] = 255;
			}

			return result;
		}

		return pixels;
	}

	void Texture2D::convert565(unsigned short pixel, byte* r, byte* g, byte* b)
	{
		int tr = ((pixel & 0xF800) >> 11);
        int tg = ((pixel & 0x07E0) >> 5);
        int tb = (pixel & 0x001F);

        // now scale the values up to max of 255
        *r = (byte)(tr * 255 / 31);
        *g = (byte)(tg * 255 / 63);
        *b = (byte)(tb * 255 / 31);
	}

	struct Color8888
	{
		byte r, g, b, a;
	};

	class Dxt1Decompressor
	{
	public:
		static byte* Convert(const byte* pixels, int width, int height, int size)
		{
			byte* output = new byte[width * height * 4];

			BlockDecompressImageDXT1(width, height, pixels, (unsigned int*)output);

			return output;
		}

	private:

		// most of the following was borrowed from
		// http://www.glassechidna.com.au/2009/devblogs/s3tc-dxt1dxt5-texture-decompression/
		static unsigned int PackRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
		{
			return ((a << 24) | (b << 16) | (g << 8) | r);
			//return ((r << 24) | (g << 16) | (b << 8) | a);
		}
 
		// void DecompressBlockDXT1(): Decompresses one block of a DXT1 texture and stores the resulting pixels at the appropriate offset in 'image'.
		//
		// unsigned long x:						x-coordinate of the first pixel in the block.
		// unsigned long y:						y-coordinate of the first pixel in the block.
		// unsigned long width: 				width of the texture being decompressed.
		// unsigned long height:				height of the texture being decompressed.
		// const unsigned char *blockStorage:	pointer to the block to decompress.
		// unsigned long *image:				pointer to image where the decompressed pixel data should be stored.
 
		static void DecompressBlockDXT1(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, unsigned int *image)
		{
			unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage);
			unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 2);
 
			unsigned long temp;
 
			temp = (color0 >> 11) * 255 + 16;
			unsigned char r0 = (unsigned char)((temp/32 + temp)/32);
			temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
			unsigned char g0 = (unsigned char)((temp/64 + temp)/64);
			temp = (color0 & 0x001F) * 255 + 16;
			unsigned char b0 = (unsigned char)((temp/32 + temp)/32);
 
			temp = (color1 >> 11) * 255 + 16;
			unsigned char r1 = (unsigned char)((temp/32 + temp)/32);
			temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
			unsigned char g1 = (unsigned char)((temp/64 + temp)/64);
			temp = (color1 & 0x001F) * 255 + 16;
			unsigned char b1 = (unsigned char)((temp/32 + temp)/32);
 
			unsigned int code = *reinterpret_cast<const unsigned int *>(blockStorage + 4);
 
			for (int j=0; j < 4; j++)
			{
				for (int i=0; i < 4; i++)
				{
					unsigned int finalColor = 0;
					unsigned char positionCode = (code >>  2*(4*j+i)) & 0x03;
 
					if (color0 > color1)
					{
						switch (positionCode)
						{
							case 0:
								finalColor = PackRGBA(r0, g0, b0, 255);
								break;
							case 1:
								finalColor = PackRGBA(r1, g1, b1, 255);
								break;
							case 2:
								finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, 255);
								break;
							case 3:
								finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, 255);
								break;
						}
					}
					else
					{
						switch (positionCode)
						{
							case 0:
								finalColor = PackRGBA(r0, g0, b0, 255);
								break;
							case 1:
								finalColor = PackRGBA(r1, g1, b1, 255);
								break;
							case 2:
								finalColor = PackRGBA((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, 255);
								break;
							case 3:
								finalColor = PackRGBA(0, 0, 0, 255);
								break;
						}
					}
 
					if (x + i < width && y + j < width)
						image[(y + j)*width + (x + i)] = finalColor;
				}
			}
		}
 
		// void BlockDecompressImageDXT1(): Decompresses all the blocks of a DXT1 compressed texture and stores the resulting pixels in 'image'.
		//
		// unsigned long width:					Texture width.
		// unsigned long height:				Texture height.
		// const unsigned char *blockStorage:	pointer to compressed DXT1 blocks.
		// unsigned long *image:				pointer to the image where the decompressed pixels will be stored.
 
		static void BlockDecompressImageDXT1(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned int *image)
		{
			unsigned long blockCountX = (width + 3) / 4;
			unsigned long blockCountY = (height + 3) / 4;
			unsigned long blockWidth = (width < 4) ? width : 4;
			unsigned long blockHeight = (height < 4) ? height : 4;
 
			for (unsigned long j = 0; j < blockCountY; j++)
			{
				for (unsigned long i = 0; i < blockCountX; i++) DecompressBlockDXT1(i*4, j*4, width, blockStorage + i * 8, image);
				blockStorage += blockCountX * 8;
			}
		}
	};

	class Dxt3Decompressor
	{
	public:
		static byte* Convert(const byte* pixels, int width, int height, int size)
		{
			byte* output = new byte[width * height * 4];

			if (DecompressDXT3(width, height, size, pixels, output))
				return output;

			delete[] output;

			return nullptr;
		}

	private:

		static void decompressDxtc(const byte* pixels, Color8888* outPixels)
		{
			byte r0, g0, b0, r1, g1, b1;

			b0 = pixels[0] & 0x1F;
			g0 = ((pixels[0] & 0xE0) >> 5) | ((pixels[1] & 0x7) << 3);
			r0 = (pixels[1] & 0xF8) >> 3;

			b1 = pixels[2] & 0x1F;
			g1 = ((pixels[2] & 0xE0) >> 5) | ((pixels[3] & 0x7) << 3);
			r1 = (pixels[3] & 0xF8) >> 3;

			outPixels[0].r = r0 << 3 | r0 >> 2;
			outPixels[0].g = g0 << 2 | g0 >> 3;
			outPixels[0].b = b0 << 3 | b0 >> 2;

			outPixels[1].r = r1 << 3 | r1 >> 2;
			outPixels[1].g = g1 << 2 | g1 >> 3;
			outPixels[1].b = b1 << 3 | b1 >> 2;
		}

		static bool DecompressDXT3(int width, int height, int size, const byte* lCompData, byte* output)
		{
			unsigned int		x, y, i, j, k, Select;
			const byte		*Temp;
			//Color565	*color_0, *color_1;
			Color8888	colours[4], *col;
			unsigned int		bitmask, Offset;
			unsigned short	word;
			const byte		*alpha;

			int pitch = width * 4;

			if (!lCompData)
				return false;

			Temp = lCompData;
			for (y = 0; y < height; y += 4) {
				for (x = 0; x < width; x += 4) {
					alpha = Temp;
					Temp += 8;
					decompressDxtc(Temp, colours);
					bitmask = ((unsigned int*)Temp)[1];
					Temp += 8;

					// Four-color block: derive the other two colors.    
					// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
					// These 2-bit codes correspond to the 2-bit fields 
					// stored in the 64-bit block.
					colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
					colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
					colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
					//colours[2].a = 0xFF;

					colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
					colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
					colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
					//colours[3].a = 0xFF;

					k = 0;
					for (j = 0; j < 4; j++) {
						for (i = 0; i < 4; i++, k++) {
							Select = (bitmask & (0x03 << k*2)) >> k*2;
							col = &colours[Select];

							if (((x + i) < width) && ((y + j) < height)) {
								Offset = (y + j) * pitch + (x + i) * 4;
								output[Offset + 0] = col->r;
								output[Offset + 1] = col->g;
								output[Offset + 2] = col->b;
							}
						}
					}

					for (j = 0; j < 4; j++) {
						word = alpha[2*j] + 256*alpha[2*j+1];
						for (i = 0; i < 4; i++) {
							if (((x + i) < width) && ((y + j) < height)) {
								Offset = (y + j) * pitch + (x + i) * 4 + 3;
								output[Offset] = word & 0x0F;
								output[Offset] = output[Offset] | (output[Offset] << 4);
							}
							word >>= 4;
						}
					}
				}
			}

			return true;
		}
	};

	byte* Texture2D::DecompressDxtc1(const byte* pixels, int width, int height, int size)
	{
		return Dxt1Decompressor::Convert(pixels, width, height, size);
	}

	byte* Texture2D::DecompressDxtc3(const byte* pixels, int width, int height, int size)
	{
		return Dxt3Decompressor::Convert(pixels, width, height, size);
	}
}
}