#! /usr/bin/runghc

import Graphics.UI.SDL
import Graphics.UI.SDL.Primitives

fontCharacterWidth = 6
fontCharacterHeight = 13

copyBlock s1 s2 w h x = blitSurface s1 r1 s2 r2
  where r1 = Just $ Rect (w * x) 0 w h
        r2 = Just $ Rect 0 (h * x) w h

main = withInit [InitVideo] $ do
  screen <- setVideoMode 800 480 32 []
  mapRGB (surfaceGetPixelFormat screen) 0 0x55 0xaa >>= fillRect screen Nothing
  -- font size fontCharacterWidth * fontCharacterHeight
  font <- loadBMP "font.bmp"
  let fontH = surfaceGetHeight font
      fontW = surfaceGetWidth font
  blitSurface font Nothing screen Nothing
  surf <- createRGBSurfaceEndian [] fontCharacterWidth (0x100 * fontCharacterHeight) 32
  surf2 <- createRGBSurfaceEndian [] 8 ((0x7f - 0x20) * fontCharacterHeight) 32
  mapRGB (surfaceGetPixelFormat surf) 0xff 0 0 >>= fillRect surf Nothing  
  mapRGB (surfaceGetPixelFormat surf2) 0xff 0 0xff >>= fillRect surf2 Nothing  
  print fontW
  print fontH
  mapM_ (copyBlock font surf fontCharacterWidth fontCharacterHeight) [0..256]
  blitSurface surf (Just $ Rect 0 (0x20 * fontCharacterHeight) fontCharacterWidth (0x17f * fontCharacterHeight)) surf2 Nothing
  blitSurface surf2 Nothing screen Nothing
  Graphics.UI.SDL.flip screen
  -- save whole font
  saveBMP surf2 "font.t.bmp"
  delay 2000