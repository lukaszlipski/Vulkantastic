import os

CurrentPath = os.path.dirname(__file__)

SrcPath = os.path.join(CurrentPath, "../Source/Renderer/vertex_definitions.h")
DstPath = os.path.join(CurrentPath, "../Source/Renderer/vertex_definitions.cpp")
Namespace = "VertexDefinition"

SrcFileName = os.path.basename(SrcPath)

SrcFile = open(SrcPath, "r")
DstFile = open(DstPath, "w")

DstFile.write("#include \"%s\"\n" % SrcFileName)
DstFile.write("namespace %s {\n" % Namespace)

CurrentStructName = ""
StartProcessing = False

Line = SrcFile.readline()
while(Line != ""):
    Line = Line.strip()
    Words = Line.split(' ')

    if not(StartProcessing):
        if Words[0] == "struct":
            CurrentStructName = Words[1]

        if Words[0] == "DECLARE_VERTEX_FORMAT()":
            StartProcessing = True
            DstFile.write("BEGIN_VERTEX_FORMAT(%s, %s)\n" % (CurrentStructName, "false"))

        if Words[0] == "DECLARE_VERTEX_FORMAT_INST()":
            StartProcessing = True
            DstFile.write("BEGIN_VERTEX_FORMAT(%s, %s)\n" % (CurrentStructName, "true"))
    else:
        if Words[0] == "};":
            StartProcessing = False
            DstFile.write("END_VERTEX_FORMAT(%s)\n" % CurrentStructName)
        else:
            if Words[0] == "DECLARE_VERTEX_FORMAT()":
                print("Error while processing %s" % SrcFileName)

            Type = Words[0]
            Variable = Words[1]
            if Variable[-1] == ";":
                Variable = Variable[:-1]
            
            DstFile.write("VERTEX_MEMBER(%s, %s, %s)\n" % (CurrentStructName, Type, Variable))


    Line = SrcFile.readline()

DstFile.write("}\n")

SrcFile.close()
DstFile.close()