import os
from subprocess import call

CurrentPath = os.path.dirname(os.path.abspath(__file__))

SrcPath = os.path.normpath(os.path.join(CurrentPath, "../Source/Shaders/"))
DstPath = os.path.normpath(os.path.join(CurrentPath, "../Shaders/"))

Files = os.listdir(SrcPath)

for File in Files:
    FileName, FileExtension = os.path.splitext(File)
    SrcPathWithName = os.path.join(SrcPath,File)
    DstPathWithName = os.path.join(DstPath, FileName + FileExtension + ".spv")
    Command = "glslangValidator.exe -V -o " + DstPathWithName + " " + SrcPathWithName
    print(Command)
    call(str(Command))
