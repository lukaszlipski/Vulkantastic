import os
import struct
import math

CurrentPath = os.path.dirname(os.path.abspath(__file__))

SrcPath = os.path.normpath(os.path.join(CurrentPath, "../Source/Meshes/"))
DstPath = os.path.normpath(os.path.join(CurrentPath, "../Meshes/"))

def CalculateTangents(Positions, TexCoords, Indicies):
    
    TriangleCount = len(Indicies)
    Tangents = []

    for i in range(0,TriangleCount, 3):
        
        Position1Index = Indicies[i] * 3
        Position2Index = Indicies[(i + 1)] * 3
        Position3Index = Indicies[(i + 2)] * 3

        Texture1Index = Indicies[i] * 2
        Texture2Index = Indicies[(i + 1)] * 2
        Texture3Index = Indicies[(i + 2)] * 2

        E1x = Positions[Position2Index] - Positions[Position1Index]
        E1y = Positions[Position2Index + 1] - Positions[Position1Index + 1]
        E1z = Positions[Position2Index + 2] - Positions[Position1Index + 2]

        E2x = Positions[Position3Index] - Positions[Position1Index]
        E2y = Positions[Position3Index + 1] - Positions[Position1Index + 1]
        E2z = Positions[Position3Index + 2] - Positions[Position1Index + 2]

        U1 = TexCoords[Texture2Index] - TexCoords[Texture1Index]
        V1 = TexCoords[Texture2Index + 1] - TexCoords[Texture1Index + 1]

        U2 = TexCoords[Texture3Index] - TexCoords[Texture1Index]
        V2 = TexCoords[Texture3Index + 1] - TexCoords[Texture1Index + 1]

        Determinant = 1 / (U1 * V2 - V1 * U2)

        V1 = V1 * -1 * Determinant
        U1 = V2 * Determinant

        X = U1 * E1x + V1 * E2x
        Y = U1 * E1y + V1 * E2y
        Z = U1 * E1z + V1 * E2z

        Length = math.sqrt(X * X + Y * Y + Z * Z)

        X = X / Length
        Y = Y / Length
        Z = Z / Length

        Tangents.extend([X, Y, Z])
        Tangents.extend([X, Y, Z])
        Tangents.extend([X, Y, Z])
    
    return Tangents




def PostprocessSubobjectAndSave(DstFile, Positions, TexCoords, Normals, Indicies):
        PositionsLength = len(Positions) 
        TexCoordsLength = len(TexCoords) 
        NormalsLength = len(Normals) 
        IndiciesLength = len(Indicies)

        Tangents = CalculateTangents(Positions, TexCoords, Indicies)
        AttributesCount = int(PositionsLength / 3)

        # Write attrbute count
        DstFile.write( struct.pack("i",AttributesCount) )

        for i in range(0, AttributesCount):
            # Write positions
            DstFile.write( struct.pack("f",Positions[ (i * 3) ]) )
            DstFile.write( struct.pack("f",Positions[ (i * 3) + 1 ]) )
            DstFile.write( struct.pack("f",Positions[ (i * 3) + 2 ]) )

            # Write texture coordinates
            DstFile.write( struct.pack("f",TexCoords[ (i * 2) ]) )
            DstFile.write( struct.pack("f",TexCoords[ (i * 2) + 1 ]) )

            # Write normals
            DstFile.write( struct.pack("f",Normals[ (i * 3) ]) )
            DstFile.write( struct.pack("f",Normals[ (i * 3) + 1 ]) )
            DstFile.write( struct.pack("f",Normals[ (i * 3) + 2 ]) )

            # Write tangents
            DstFile.write( struct.pack("f",Tangents[ (i * 3) ]) )
            DstFile.write( struct.pack("f",Tangents[ (i * 3) + 1 ]) )
            DstFile.write( struct.pack("f",Tangents[ (i * 3) + 2 ]) )

        # Write indicies
        DstFile.write( struct.pack("i",IndiciesLength) ) # Write size
        for i in range(0, IndiciesLength):
            DstFile.write( struct.pack("I",Indicies[i]) )


def CreateSubobjectObj(DstFile, Indicies, Positions, TexCoords, Normals):
    # 1) Preprocess current mesh
    if len(Indicies) > 0:
        
        CurrentIndex = 0

        SortedPositions = []
        SortedTexCoords = []
        SortedNormals = []
        SortedIndicies = []

        ExistingIndicies = {} 
        for i in range(0, len(Indicies)):
            
            (PositionIndex, TexCoordIndex, NormalIndex) = Indicies[i].split('/')

            if PositionIndex == "" or TexCoordIndex == "" or NormalIndex == "":
                return False

            PositionIndex = (int(PositionIndex) - 1) * 3
            TexCoordIndex = (int(TexCoordIndex) - 1) * 2
            NormalIndex = (int(NormalIndex) - 1) * 3

            Key = (PositionIndex, TexCoordIndex, NormalIndex)

            if ExistingIndicies.get( Key ) == None:
                SortedPositions.extend( [Positions[PositionIndex], Positions[PositionIndex + 1], Positions[PositionIndex + 2]] )
                SortedTexCoords.extend( [TexCoords[TexCoordIndex], TexCoords[TexCoordIndex + 1]] )
                SortedNormals.extend( [Normals[NormalIndex], Normals[NormalIndex + 1], Normals[NormalIndex + 2]] )

                SortedIndicies.append(CurrentIndex)
                ExistingIndicies[Key] = CurrentIndex

                CurrentIndex = CurrentIndex + 1
            else:
                ExistingIndex = ExistingIndicies[Key]
                SortedIndicies.append(ExistingIndex)

        PostprocessSubobjectAndSave(DstFile, SortedPositions, SortedTexCoords, SortedNormals, SortedIndicies)

        # Start new one
        Indicies.clear()
    return True


def PreprocessObj(fileName):

    SrcPathWithName = os.path.join(SrcPath,fileName)

    fileNameWithoutExt = os.path.splitext(fileName)[0]

    DstPathWithName = os.path.join(DstPath,fileNameWithoutExt + ".sm")
    
    SrcFile = open(SrcPathWithName, "r")
    DstFile = open(DstPathWithName, "wb")

    Positions = []
    TexCoords = []
    Normals = []
    Indicies = []

    Line = SrcFile.readline()
    while(Line != ""):
        Line = Line.strip()
        Words = Line.split(' ')

        Type = Words[0]

        if Type == "usemtl": # New material id == new subobject
            if not(CreateSubobjectObj(DstFile, Indicies, Positions, TexCoords, Normals)):
                DstFile.close()
                os.remove(DstPathWithName)
                print("Cannot process " + fileName)
        
        elif Type == "v": # Position
            Positions.extend([float(Words[1]), float(Words[2]), float(Words[3])])

        elif Type == "vt": # Texture coordinates
            TexCoords.extend([float(Words[1]), float(Words[2])])

        elif Type == "vn": # Normal vector
            Normals.extend([float(Words[1]), float(Words[2]), float(Words[3])])

        elif Type == "f": # Indicies
            
            Indicies.extend([Words[1], Words[2], Words[3]])
            if len(Words) > 4: # Quad
                Indicies.extend([Words[1], Words[3], Words[4]])  

        Line = SrcFile.readline()

    if not(CreateSubobjectObj(DstFile, Indicies, Positions, TexCoords, Normals)):
        DstFile.close()
        os.remove(DstPathWithName)
        print("Cannot process " + fileName)

    SrcFile.close()
    DstFile.close()
            
# Start
DirExists = os.path.isdir(SrcPath)
if DirExists:
    Files = os.listdir(SrcPath)

    for File in Files:
        if File.endswith('.obj'):
            PreprocessObj(File)
        else:
            print("Unsupported file extension: " + File)
