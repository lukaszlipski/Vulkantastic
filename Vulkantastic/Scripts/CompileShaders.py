import Common
import os
import subprocess

SrcPath = Common.GetSourceFolderPath('Shaders')
DstPath = Common.GetDestinationFolderPath('Shaders')

Common.PrintHeader('Shader compiler')

Files = os.listdir(SrcPath)
for File in Files:
    FileName, FileExtension = os.path.splitext(File)
    SrcPathWithName = os.path.join(SrcPath, File)
    DstPathWithName = os.path.join(DstPath, FileName + FileExtension + ".spv")
    Command = "glslangValidator.exe -V -o " + DstPathWithName + " " + SrcPathWithName

    Common.PrintLog('Processing: %s' % FileName + FileExtension)

    process = subprocess.run(str(Command), stdout=subprocess.PIPE)

    output = process.stdout.decode('utf-8')
    output = output.split('\n')
    filteredText = list(filter(lambda x : 'error' in x or 'ERROR' in x, output))

    if len(filteredText): print('\n'.join(filteredText))

Common.PrintFooter('Shader compiler')
