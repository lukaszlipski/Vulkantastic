import os
from subprocess import call

CurrentPath = os.path.dirname(__file__)

SrcPath = os.path.join(CurrentPath, "../Source/Shaders/")
DstPath = os.path.join(CurrentPath, "../Shaders/")

Files = os.listdir(SrcPath)

for File in Files:
    FileName, FileExtension = os.path.splitext(File)
    SrcPathWithName = SrcPath + File
    DstPathWithName = DstPath + FileName + FileExtension + ".spv "
    Command = "glslangValidator.exe -V -o " + DstPathWithName + SrcPathWithName
    call(Command)
