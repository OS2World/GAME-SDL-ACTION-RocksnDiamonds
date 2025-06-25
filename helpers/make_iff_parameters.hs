#! /usr/bin/runghc

import Data.Binary.Put
import Data.IFF
import qualified Data.ByteString as B
import qualified Data.ByteString.Char8 as BC
import qualified Data.ByteString.Lazy as BL

hotCol = [ [ 0x00, 0x00, 0x00 ]
         , [ 0xff, 0x00, 0x00 ]
         , [ 0xff, 0x00, 0x00 ]
         , [ 0xff, 0x00, 0x00 ]
         , [ 0xff, 0x00, 0x00 ]
         , [ 0xff, 0xff, 0x00 ]
         , [ 0xff, 0xff, 0x00 ]
         , [ 0xff, 0xff, 0x00 ]
         , [ 0xff, 0xff, 0x00 ]
         , [ 0xfc, 0xfc, 0xfc ] ]


coolCol = [ [ 0x00, 0x02, 0x0b ]
          , [ 0x00, 0x00, 0x5b ]
          , [ 0x00, 0x00, 0xab ]
          , [ 0x00, 0xa0, 0x70 ]
          , [ 0x00, 0xd0, 0xd0 ]
          , [ 0xdc, 0xf0, 0xf0 ] ]
  
chunkHot = Cons (chunkIdFromString "CMAP") $ Chunk $ B.pack $ concat hotCol
chunkCool = Cons (chunkIdFromString "CMAP") $ Chunk $ B.pack $ concat coolCol
chunkMDOT = Cons (chunkIdFromString "MDOT") $ Chunk $ B.concat $ BL.toChunks $ runPut f
  where f = do putWord32be 0x1f040
               putWord16be 0x0000

formSpacedots = Cons (chunkIdFromString "SPDT") $ Form [ chunkSparkleNDOT ]

formSparkles = Cons (chunkIdFromString "SPRK") $ Form [ chunkHot, chunkCool, chunkMDOT ]
chunkSparkleNDOT  = Cons (chunkIdFromString "NDOT") $ Chunk $ B.concat $ BL.toChunks $ runPut f
  where f = do putWord16be 0x1003

chunkYellifish = Cons (chunkIdFromString "YLLI") $ Chunk $ B.concat $ BL.toChunks $ runPut f
  where f = do putWord16be 8
               putWord32be 0x70b20f
               putWord16be 3
               putWord16be 21
               putWord16be 18
               putWord16be 222
               putWord32be 1173957729 -- (1 <<32)*.2733333337 
               putWord32be 1301505239 -- 4.84848484/16*(1<<32)
               putWord16be 9
               putWord16be 19
-- formYellifish = Cons (chunkIdFromString "YLLI") $ Form [ chunkFish ]

chunkBlubatParameter =  Cons (chunkIdFromString "PARA") $ Chunk $ B.concat $ BL.toChunks $ runPut $ do
                          putWord16be 5

chunkAttackProbability = Cons (chunkIdFromString "APBB") $ Chunk $ B.concat $ BL.toChunks $ runPut $ do
                           putWord16be 84
formBlubats = Cons (chunkIdFromString "BBAT") $ Form [ chunkBlubatParameter, chunkAttackProbability ]

chunkAnnotation = Cons (chunkIdFromString "ANNO") $ Chunk $ BC.pack "Rockdodger parameters..."
chunkAnnotation2 = Cons (chunkIdFromString "ANNO") $ Chunk $ BC.pack "I should put more information into this file!"

chunkEdition = Cons (chunkIdFromString "EDTN") $ Chunk $ BC.pack "YELLIFISH"

chunkInfoscreen = Cons (chunkIdFromString "IFSC") $ Chunk $ B.concat $ BL.toChunks $ runPut f
  where f = do putWord8 1
               putWord8 0
               putWord16be 38

formIntro = Cons (chunkIdFromString "INTR") $ Form [ Cons (chunkIdFromString "TEXT") $ Chunk $ BC.pack txt
            ]
    where txt = "A long hiatus to this code... But work needed to be done to keep this little game in Debian. Old code and no motivation to clean it up. Maybe next time I will write some more. Stay Tuned! And... dodge the rocks!           "

chunkMoodItem = Cons (chunkIdFromString "MOOD") $ Chunk $ B.concat $ BL.toChunks $ runPut f
    where f = do putWord16be 29179
                 putWord16be 0x028B

main = B.putStr $ toByteString $ Cons (chunkIdFromString "ROCK") $ Form chunks
  where chunks = [ Cons (chunkIdFromString "NAME") $ Chunk $ BC.pack "Rockdodger"
                 , chunkAnnotation
                 , formIntro
                 , formSparkles
                 , formSpacedots
                 , chunkYellifish
                 , formBlubats
                 , chunkEdition
                 , chunkInfoscreen
                 , chunkMoodItem
                 , chunkAnnotation2
                 ]

