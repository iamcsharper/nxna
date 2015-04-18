Nxna ("Not XNA") is an implementation of the Microsoft XNA 4.0 Framework in unmanaged C++. It currently uses OpenGL (or OpenGL ES on certain platforms) for graphics, and work has begun on an OpenAL back-end for sound. It is intended to duplicate the XNA API as closely as possible.

There are some differences between XNA and NXNA:

  * XNA supports programmable shaders in HLSL. NXNA only currently supports the built-in shaders (it may be possible to write your own GLSL shaders and use them, but you have to use the "hacky path"). Hopefully there will someday be some kind of HLSL compiler built-in.
  * Setting up the video device is much more primitive in NXNA. Currently you have to call GraphicsDeviceManager::SetScreenSize(). ApplyChanges() doesn't work.
  * NXNA does not have its own content pipeline mechanism. It expects all resources to be already built by the XNA content pipeline (in other words, it expects all content to be in .xnb files)
  * NXNA includes a SpriteSheet class, which is based on the SpriteSheet sample (http://create.msdn.com/en-US/education/catalog/sample/sprite_sheet), although the .xnb files the code from the SpriteSheet sample generates are incompatible with the NXNA version.

# Building #

NXNA currently builds and runs (although there's always a chance that support for a particular platform is broken at any given time) on Windows, Linux, MacOSX, and iOS.

NXNA depends on SDL 2.0 (on Linux and OSX), Glew (on non-OpenGL ES platforms), and OpenAL.

## Windows ##
Just open the nxna.vcxproj in Visual C++ 2010 and build.

## Linux ##
First, build SDL 2.0. It's in the lib/SDL\_13 folder. Then use the genMakefile script to generate a makefile, then build using the generated makefile.

## MacOSX ##
First, build SDL 2.0. It's in the lib/SDL\_13 folder. Then use the genCMake script to generate a CMake script, then use that script to generate an Xcode project, then build with Xcode.

# Using NXNA #
Take a look at the NxnaHelloWorld project inside the tests folder. It contains a very simple project that you can expand upon.