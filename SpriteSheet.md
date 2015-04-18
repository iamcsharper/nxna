# Sprite Sheet #
NXNA provides a built-in version of the Sprite Sheet (http://create.msdn.com/en-US/education/catalog/sample/sprite_sheet), but you'll need to use our custom version of the Sprite Sheet pipeline and content reader.

Steps:
  1. Download and extract the sample
  1. Apply the patch available at the bottom of this page
  1. Rebuild

That's it.

## Patch ##
```
diff -rupN old\/SpriteSheetPipeline/SpriteSheetPipeline.csproj new\/SpriteSheetPipeline/SpriteSheetPipeline.csproj
--- old\/SpriteSheetPipeline/SpriteSheetPipeline.csproj	2010-09-13 12:40:48.000000000 -0500
+++ new\/SpriteSheetPipeline/SpriteSheetPipeline.csproj	2011-12-15 11:06:52.000000000 -0600
@@ -1,7 +1,7 @@
 ï»¿<?xml version="1.0" encoding="utf-8"?>
 <Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003" ToolsVersion="4.0">
   <PropertyGroup>
-    <ProjectGuid>{25146084-89AA-4A55-8D32-E237CBA223C0}</ProjectGuid>
+    <ProjectGuid>{288693A4-E777-4A32-A658-3AB8B48C8511}</ProjectGuid>
     <Configuration Condition=" '$(Configuration)' == '' ">Debug</Configuration>
     <Platform Condition=" '$(Platform)' == '' ">x86</Platform>
     <OutputType>Library</OutputType>
@@ -45,6 +45,7 @@ ï»¿<?xml version="1.0" encoding="utf-8"?
     <Compile Include="SpritePacker.cs" />
     <Compile Include="SpriteSheetContent.cs" />
     <Compile Include="SpriteSheetProcessor.cs" />
+    <Compile Include="SpriteSheetWriter.cs" />
   </ItemGroup>
   <Import Project="$(MSBuildBinPath)\Microsoft.CSharp.targets" />
   <Import Project="$(MSBuildExtensionsPath)\Microsoft\XNA Game Studio\Microsoft.Xna.GameStudio.ContentPipelineExtensions.targets" />
diff -rupN old\/SpriteSheetPipeline/SpriteSheetWriter.cs new\/SpriteSheetPipeline/SpriteSheetWriter.cs
--- old\/SpriteSheetPipeline/SpriteSheetWriter.cs	1969-12-31 18:00:00.000000000 -0600
+++ new\/SpriteSheetPipeline/SpriteSheetWriter.cs	2011-10-31 15:07:10.000000000 -0500
@@ -0,0 +1,47 @@
+using System;
+using System.Collections.Generic;
+using System.Linq;
+using System.Text;
+using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Compiler;
+
+using TWrite = SpriteSheetPipeline.SpriteSheetContent;
+using System.Reflection;
+
+namespace SpriteSheetPipeline
+{
+    [ContentTypeWriter]
+    class SpriteSheetWriter : ContentTypeWriter<TWrite>
+    {
+        protected override void Write(ContentWriter output, TWrite value)
+        {
+            // we need access to the TextureWriter, but it's internal, so we have to break
+            // the rules and use reflection... muaah haa haa haaah!
+            Assembly s = Assembly.Load("Microsoft.Xna.Framework.Content.Pipeline");
+            Type t = s.GetType("Microsoft.Xna.Framework.Content.Pipeline.Serialization.Compiler.Texture2DWriter");
+            MethodInfo m = t.GetMethod("Write", BindingFlags.NonPublic | BindingFlags.Instance, null, 
+                new Type[] { typeof(ContentWriter), typeof(Microsoft.Xna.Framework.Graphics.Texture2D) }, null);
+            object writer = Activator.CreateInstance(t);
+
+            m.Invoke(writer, new object[] { output, value.Texture });
+
+            // now write the rectangles
+            output.Write(value.SpriteRectangles.Count);
+
+            foreach (var r in value.SpriteRectangles)
+            {
+                output.Write(r.X);
+                output.Write(r.Y);
+                output.Write(r.Width);
+                output.Write(r.Height);
+            }
+        }
+
+        public override string GetRuntimeReader(Microsoft.Xna.Framework.Content.Pipeline.TargetPlatform targetPlatform)
+        {
+            if (targetPlatform == Microsoft.Xna.Framework.Content.Pipeline.TargetPlatform.Windows)
+                return "SpriteSheetRuntime.SpriteSheetReader, SpriteSheetRuntime";
+            else
+                return "SpriteSheetRuntime.SpriteSheetReader, SpriteSheetRuntime";
+        }
+    }
+}
diff -rupN old\/SpriteSheetRuntime/SpriteSheet.cs new\/SpriteSheetRuntime/SpriteSheet.cs
--- old\/SpriteSheetRuntime/SpriteSheet.cs	2010-09-13 12:40:48.000000000 -0500
+++ new\/SpriteSheetRuntime/SpriteSheet.cs	2011-12-15 10:22:40.000000000 -0600
@@ -32,7 +32,7 @@ namespace SpriteSheetRuntime
 
         // Remember where in the texture each sprite has been placed.
         [ContentSerializer]
-        List<Rectangle> spriteRectangles = null;
+        List<Rectangle> spriteRectangles = new List<Rectangle>();
 
         // Store the original sprite filenames, so we can look up sprites by name.
         [ContentSerializer]
@@ -45,6 +45,7 @@ namespace SpriteSheetRuntime
         public Texture2D Texture
         {
             get { return texture; }
+            internal set { texture = value; }
         }
 
         
@@ -70,6 +71,15 @@ namespace SpriteSheetRuntime
             return spriteRectangles[spriteIndex];
         }
 
+        internal List<Rectangle> Rectangles
+        {
+            get { return spriteRectangles; }
+        }
+
+        public int NumSprites
+        {
+            get { return spriteRectangles.Count; }
+        }
 
         /// <summary>
         /// Looks up the numeric index of the specified sprite. This is useful when
diff -rupN old\/SpriteSheetRuntime/SpriteSheetReader.cs new\/SpriteSheetRuntime/SpriteSheetReader.cs
--- old\/SpriteSheetRuntime/SpriteSheetReader.cs	1969-12-31 18:00:00.000000000 -0600
+++ new\/SpriteSheetRuntime/SpriteSheetReader.cs	2011-10-31 15:07:02.000000000 -0500
@@ -0,0 +1,49 @@
+using System;
+using System.Collections.Generic;
+using System.Linq;
+using System.Text;
+using Microsoft.Xna.Framework.Content;
+using Microsoft.Xna.Framework.Graphics;
+
+namespace SpriteSheetRuntime
+{
+    class SpriteSheetReader : ContentTypeReader<SpriteSheet>
+    {
+        protected override SpriteSheet Read(ContentReader input, SpriteSheet existingInstance)
+        {
+            SpriteSheet sheet = existingInstance;
+
+            if (sheet == null)
+                sheet = new SpriteSheet();
+            
+            IGraphicsDeviceService graphicsDeviceService = (IGraphicsDeviceService)input.ContentManager.ServiceProvider.GetService(typeof(IGraphicsDeviceService));
+
+            SurfaceFormat format = (SurfaceFormat)input.ReadInt32();
+            int width = input.ReadInt32();
+            int height = input.ReadInt32();
+            int num = input.ReadInt32();
+            sheet.Texture = new Texture2D(graphicsDeviceService.GraphicsDevice, width, height, num > 1, format);
+            for (int i = 0; i < num; i++)
+            {
+                int num2 = input.ReadInt32();
+                byte[] data = input.ReadBytes(num2);
+                sheet.Texture.SetData<byte>(i, null, data, 0, num2);
+            }
+
+            // read the rects
+            int numRects = input.ReadInt32();
+            for (int i = 0; i < numRects; i++)
+            {
+                Microsoft.Xna.Framework.Rectangle r = new Microsoft.Xna.Framework.Rectangle();
+                r.X = input.ReadInt32();
+                r.Y = input.ReadInt32();
+                r.Width = input.ReadInt32();
+                r.Height = input.ReadInt32();
+
+                sheet.Rectangles.Add(r);
+            }
+
+            return sheet;
+        }
+    }
+}
diff -rupN old\/SpriteSheetRuntime/SpriteSheetRuntime (Windows).csproj new\/SpriteSheetRuntime/SpriteSheetRuntime (Windows).csproj
--- old\/SpriteSheetRuntime/SpriteSheetRuntime (Windows).csproj	2010-09-13 12:40:48.000000000 -0500
+++ new\/SpriteSheetRuntime/SpriteSheetRuntime (Windows).csproj	2011-12-15 10:49:26.000000000 -0600
@@ -87,6 +87,7 @@ ï»¿<?xml version="1.0" encoding="utf-8"?
   <ItemGroup>
     <Compile Include="Properties\AssemblyInfo.cs" />
     <Compile Include="SpriteSheet.cs" />
+    <Compile Include="SpriteSheetReader.cs" />
   </ItemGroup>
   <ItemGroup>
     <Reference Include="Microsoft.Xna.Framework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=842cf8be1de50553, processorArchitecture=x86" />

```