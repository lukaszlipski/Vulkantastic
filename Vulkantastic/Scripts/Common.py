import os

def PrintHeader(scriptName):
    print(' ')
    print('----------------------------------------------- ' + scriptName + ' -----------------------------------------------')

def PrintFooter(scriptName):
    print(' ')

def PrintLog(msg):
    print(msg)

def GetSourceFolderPath(srcFolder):
    CurrentPath = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(CurrentPath, '../Source/' + srcFolder + '/'))

def GetDestinationFolderPath(dstFolder):
    CurrentPath = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(CurrentPath, '../' + dstFolder + '/'))