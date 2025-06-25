
module Main where

import Control.Monad
import Data.Word
import Graphics.UI.SDL
import Graphics.UI.SDL.Image
import System.Environment (getArgs)

loadImage :: String -> IO Surface
loadImage fname = load fname >>= displayFormat

loadImageCK :: String -> Maybe (Word8, Word8, Word8) -> IO Surface
loadImageCK fname colKey = loadImage fname >>= setColorKey' colKey

setColorKey' :: Maybe (Word8, Word8, Word8) -> Surface -> IO Surface
setColorKey' Nothing s = return s
setColorKey' (Just (r, g, b)) s = (mapRGB . surfaceGetPixelFormat) s r g b >>= setColorKey s [SrcColorKey] >> return s

applySurface :: Int -> Int -> Surface -> Surface -> IO Bool
applySurface x y src dst = blitSurface src Nothing dst offset
  where offset = Just Rect { rectX = x, rectY = y, rectW = 0, rectH = 0 }

parseColKey :: [String] -> Maybe (Word8, Word8, Word8)
parseColKey [r, g, b] = Just (read r, read g, read b)
parseColKey _ = Nothing

main = withInit [] $ do
  (fname:args) <- getArgs
  screen <- setVideoMode 800 600 32 []
  setCaption "Display Image" []
  image <- loadImage fname
  imageck <- loadImageCK fname $ parseColKey args
  applySurface 0 0 image screen
  applySurface (surfaceGetWidth image + 5) 0 imageck screen
  applySurface 0 (surfaceGetHeight image + 5) imageck screen
  Graphics.UI.SDL.flip screen
  loop
  where
    loop = do
      delay 200
      quit <- whileEvents
      unless quit loop
    whileEvents = do
      event <- pollEvent
      case event of
        Quit -> return True
        NoEvent -> return False
        _ -> whileEvents
