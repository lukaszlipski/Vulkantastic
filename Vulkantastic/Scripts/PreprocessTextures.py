import Common
import os
import struct
import math
from enum import Enum

SrcPath = Common.GetSourceFolderPath('Textures')
DstPath = Common.GetDestinationFolderPath('Textures')

# Read tga
class TgaLoader:
    def __init__(self, path):
        self.Path = path
        self.ReadFile()

    def ReadFile(self):
        
        file = open(self.Path, mode='rb')
        data = file.read()
        self.ProcessHeader(data)
        self.ProcessPixels(data)
        file.close()
        
    def ProcessHeader(self, data):

        (self.IdLen, self.ColorMapType, self.ImageType) = struct.unpack_from('=bbb', data, 0)
        (self.FirstEntryIndex, self.ColorMapLength, self.ColorMapEntrySize) = struct.unpack_from('=hhb', data, 3)
        (self.xOrigin, self.yOrigin, self.Width, self.Height, self.PixelDepth, self.PixelDesc) = struct.unpack_from('=hhhhbb', data, 8)

    def ColorMapOffset(self):
        return self.IdLen + self.ColorMapLength

    def HeaderOffset(self):
        return 18

    def GetPixels(self):
        return self.Pixels

    def GetBytesPerPixel(self):
        return self.PixelDepth / 8

    def GetWidth(self):
        return self.Width

    def GetHeight(self):
        return self.Height

    def ProcessPixels(self, data):

        assert(self.ColorMapType == 0) # Currently, color maps are not supported

        offset = self.ColorMapOffset() + self.HeaderOffset()
        
        if(self.ImageType >= 9):
            self.Pixels = self.__DecodePixels(data, offset)
        else:
            self.Pixels = []
            format = self.__GetFormatByPixelDepth()
            for index in range(self.Width * self.Height):
                currentOffset = int(offset + (index * self.GetBytesPerPixel()))
                pixel = struct.unpack_from(format, data, currentOffset)
                self.Pixels.extend(pixel)

        self.__ReorderBytes()
        self.__ReorderPixels()
        

    def __ReorderBytes(self):

        bytesPerPixel = self.GetBytesPerPixel()

        if(bytesPerPixel == 4 or bytesPerPixel == 3):
            def ReorderBytesInternal(index): 
                self.Pixels[index], self.Pixels[index + 2] = self.Pixels[index + 2], self.Pixels[index]
        elif(bytesPerPixel == 2):
            def ReorderBytesInternal(index): 
                self.Pixels[index], self.Pixels[index + 1] = self.Pixels[index + 1], self.Pixels[index]
        else:
            def ReorderBytesInternal(index): 
                pass

        for y in range( self.Height ):
            for x in range( self.Width ):
                index = ((self.Height * y) + x) * bytesPerPixel
                ReorderBytesInternal( int(index) )

    def __ReorderPixels(self):

        imgOrigin = (self.PixelDesc >> 4) 

        # We want to have an image that starts in a left top corner
        reverseX = imgOrigin & 0x1
        reverseY = not(imgOrigin & 0x2)
        
        bytesPerPixel = self.GetBytesPerPixel()

        if reverseY:
            for index in range( int(self.Height / 2) ):
                posAStart = int( index * self.Width * bytesPerPixel )
                posAEnd = int(posAStart + self.Width * bytesPerPixel)
                posBStart = int( (self.Height - 1 - index) * self.Width * bytesPerPixel )
                posBEnd = int(posBStart + self.Width * bytesPerPixel)
                self.Pixels[posAStart : posAEnd], self.Pixels[posBStart : posBEnd] = self.Pixels[posBStart : posBEnd], self.Pixels[posAStart : posAEnd]

        assert(not(reverseX)) # Reversing on X has not been implemented yet


    def __DecodePixels(self, data, dataOffset):

        result = [] # decoded pixels
        exitCondition = self.Width * self.Height * self.GetBytesPerPixel()
        format = self.__GetFormatByPixelDepth()

        offset = 0
        while len(result) < exitCondition:
            
            packetsHeader = struct.unpack_from('=B', data, int( dataOffset + offset) )[0]
            offset = offset + 1
            
            isRunTimeEncoded = packetsHeader >> 7
            numberOfPixels = (packetsHeader & 0x7F) + 1 # There is always one more pixel than specified by those 7 bits

            if(isRunTimeEncoded):
                currentOffset = int(dataOffset + offset)
                packetsValue = struct.unpack_from(format, data, currentOffset)
                result.extend( [*packetsValue] * numberOfPixels )

                offset = offset + self.GetBytesPerPixel()
            else:
                
                for index in range(numberOfPixels):
                    currentOffset = int(dataOffset + offset + ( index * self.GetBytesPerPixel() ) )
                    packetsValue = struct.unpack_from(format, data, currentOffset )
                    result.extend( [*packetsValue] )

                offset = offset + self.GetBytesPerPixel() * numberOfPixels
                
        return result

    # Chooses proper format based on the number of bytes per pixel
    def __GetFormatByPixelDepth(self):
        if self.GetBytesPerPixel() == 1:
            return '=B'
        elif self.GetBytesPerPixel() == 2:
            return '=BB'
        elif self.GetBytesPerPixel() == 3:
            return '=BBB'
        elif self.GetBytesPerPixel() == 4:
            return '=BBBB'

class Format(Enum):
    R8G8B8A8_UNORM = 28
    R8G8B8A8_UNORM_SRGB = 29
    BC1_UNORM = 71

class Dimension(Enum):
    TEXTURE1D = 2
    TEXTURE2D = 3
    TEXTURE3D = 4

class DDSHeaderDX10:
    def __init__(self, Format, Type):
        self.Format = Format
        self.Type = Type

    def Get(self):
        header = bytearray(5 * 4)
        struct.pack_into('=I', header, 0 * 4, self.Format.value) # Format
        struct.pack_into('=I', header, 1 * 4, self.Type.value) # Dimension
        # Is cubemap
        # Array size
        # Flags

        return header


class DDSPixelFormat:

    def __init__(self, BytesPerPixel, IsCompressed = False):
        self.BytesPerPixel = BytesPerPixel
        self.IsCompressed = IsCompressed

    def Get(self):
        format = bytearray(32)
        struct.pack_into('=I', format, 0 * 4, 32) # Size
        struct.pack_into('=I', format, 1 * 4, self.__GetFlags()) # Flags
        struct.pack_into('=I', format, 2 * 4, 0x44583130) # FourCC
        struct.pack_into('=I', format, 3 * 4, 0) # RGB bit count
        struct.pack_into('=I', format, 4 * 4, 0xff000000) # R mask
        struct.pack_into('=I', format, 5 * 4, 0x00ff0000 if self.BytesPerPixel > 1 else 0) # G mask
        struct.pack_into('=I', format, 6 * 4, 0x0000ff00 if self.BytesPerPixel > 2 else 0) # B mask
        struct.pack_into('=I', format, 7 * 4, 0x000000ff if self.BytesPerPixel > 3 else 0) # A mask

        return format


    def __GetFlags(self):

        flags = 0x0

        if self.IsCompressed:
            flags = flags | 0x4
        else:
            flags = flags | 0x40

        if self.BytesPerPixel == 4: # Has alpha
            flags = flags | 0x2

        return flags


class DDSHeader:

    def __init__(self, Width, Height, BytesPerPixel, Format, Type, MipMapCount = 0):
        self.Width = Width
        self.Height = Height
        self.BytesPerPixel = BytesPerPixel
        self.Format = Format
        self.Type = Type
        self.MipMapCount = MipMapCount

    def Get(self):
        header = bytearray(124 + 4) # Plus 4 for a magic number
        struct.pack_into('=I', header, 0 * 4, 0x20534444) # DDS magic number

        # Header
        struct.pack_into('=I', header, 1 * 4, 124) # Size
        struct.pack_into('=I', header, 2 * 4, self.__GetHeaderFlags()) # Flags
        struct.pack_into('=I', header, 3 * 4, self.Height) # Height
        struct.pack_into('=I', header, 4 * 4, self.Width) # Width
        struct.pack_into('=I', header, 5 * 4, 0) # Pitch #TODO
        struct.pack_into('=I', header, 6 * 4, 0) # Depth 
        struct.pack_into('=I', header, 7 * 4, self.MipMapCount) # Mipmap count 
        # Reserved

        pixelFormat = DDSPixelFormat(self.BytesPerPixel, False)
        pixelFormatStart = 8 * 4 + 4 * 11
        header[ pixelFormatStart : pixelFormatStart + 8 * 4 ] = pixelFormat.Get() # PixelFormat

        struct.pack_into('=I', header, 8 * 4 + 4 * 11 + 4 * 8, self.__GetCapsFlags()) # Caps
        # Caps2
        # Unused

        # DX10 Header
        dx10Header = DDSHeaderDX10(self.Format, self.Type)
        header.extend(dx10Header.Get())

        return header

    def __GetHeaderFlags(self):

        hasMipMaps = self.MipMapCount > 0

        flags = 0x1 | 0x2 | 0x4 | 0x1000 # Required
        if hasMipMaps:
            flags = flags | 0x20000

        return flags

    def __GetCapsFlags(self):

        hasMipMaps = self.MipMapCount > 0

        flags = 0x1000 # Required

        if hasMipMaps:
            flags = flags | 0x8 | 0x400000

        return flags


class ICompression:
    def __init__(self, img, isSRGB):
        self.Image = img
        self.IsSRGB = isSRGB

    def CalculateMipMapCount(self):
        return math.floor(math.log2(max(self.Image.GetWidth(), self.Image.GetHeight()))) + 1


class BlockCompression0(ICompression):

    def __init__(self, img, isSRGB):
        super().__init__(img, isSRGB)

    def Compress(self):
        pass

    def GetData(self):

        header = DDSHeader(self.Image.GetWidth(), self.Image.GetHeight(), self.Image.GetBytesPerPixel(), self.GetFormat(), Dimension.TEXTURE2D, 0)
        
        data = bytearray()
        data.extend(header.Get())

        pixels = self.Image.GetPixels()

        for elem in pixels:         
            data.extend(struct.pack('=B', elem))

        return data

    def GetFormat(self):
        bytesPerFormat = self.Image.GetBytesPerPixel()
        if bytesPerFormat == 4:
            return Format.R8G8B8A8_UNORM_SRGB if self.IsSRGB else Format.R8G8B8A8_UNORM
        
        assert(False) # Unsupported format



        
class BlockCompression1:

    def __init__(self, img, isSRGB):
        self.Image = img
        self.IsSRGB = isSRGB
        
    def Compress(self):

        self.CompressedImage = []

        width = self.Image.GetWidth()
        height = self.Image.GetHeight()

        completeBlocksX = math.floor( width / 4 )
        completeBlocksY = math.floor( height / 4 )

        incompleteBlockX = math.ceil( (width % 4) / 4 )
        incompleteBlockY = math.ceil( (height % 4) / 4 )

        for y in range(completeBlocksY):
            for x in range(completeBlocksX):
                pass # TODO

        




# Start

Common.PrintHeader('Texture preprocessor')

DirExists = os.path.isdir(SrcPath)
if DirExists:
    Files = os.listdir(SrcPath)


    for File in Files:
        if File.endswith('.tga'):
            
            Common.PrintLog('Processing: %s' % File)

            fileWithoutExt = File[ 0 : File.rfind('.') ]

            srcFilePath = os.path.join(SrcPath, File)
            loader = TgaLoader(srcFilePath)

            compression = BlockCompression0(loader, True)
            compression.Compress()
            data = compression.GetData()

            dstFilePath = os.path.join(DstPath, fileWithoutExt + '.dds')
            fileToWrite = open(dstFilePath, 'wb')
            fileToWrite.write(data)
            fileToWrite.close()

        else:
            Common.PrintLog('Unsupported file extension: ' + File)

Common.PrintFooter('Texture preprocessor')
