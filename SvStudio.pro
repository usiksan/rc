#-------------------------------------------------
#
# Project created by QtCreator 2016-01-12T21:58:19
#
#-------------------------------------------------

QT       += core gui network serialport

CONFIG += c++17


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = SvStudio
TEMPLATE = app




RESOURCES += \
    SvStudio/SvRes.qrc

FORMS += \
    SvStudio/DPrjProp.ui \
    SvStudio/DProcess.ui \
    SvStudio/DTextEditorSettings.ui


HEADERS += \
  SvCompiler/SvCompiler.h \
  SvCompiler/SvVpuCompiler.h \
  SvHost/SvComBook.h \
  SvHost/SvDir.h \
  SvHost/SvMirror.h \
  SvHost/SvMirrorCom.h \
  SvHost/SvMirrorExtern.h \
  SvHost/SvMirrorLocal.h \
  SvHost/SvMirrorManager.h \
  SvHost/SvMirrorRemote.h \
  SvHost/SvProgramm.h \
  SvHost/SvTextStreamIn.h \
  SvHost/SvTextStreamOut.h \
  SvHost/SvVMachineLocal.h \
  SvNet/SvNetBlockInfo.h \
  SvNet/SvNetChannel.h \
  SvNet/SvNetHandler.h \
  SvNet/SvNetHandlerFile.h \
  SvNet/SvNetHandlerMirror.h \
  SvNet/SvNetMirror.h \
  SvNet/SvNetServer.h \
  SvStudio/DNetProcess.h \
  SvStudio/DNetProcessFileDirRequest.h \
  SvStudio/DNetProcessFileOperation.h \
  SvStudio/DNetProcessFileRequest.h \
  SvStudio/DNetProcessFileSend.h \
  SvStudio/DPrjProp.h \
  SvStudio/DProcess.h \
  SvStudio/DTextEditorSettings.h \
  SvStudio/Highlighter.h \
  SvStudio/IngDebugCalculator.h \
  SvStudio/SvConfig.h \
  SvStudio/SvPeriodicParser.h \
  SvStudio/SvProject.h \
  SvStudio/SvUtils.h \
  SvStudio/WBrowser.h \
  SvStudio/WCChartLegend.h \
  SvStudio/WCModeBoard.h \
  SvStudio/WCModeEditor.h \
  SvStudio/WCModeHelp.h \
  SvStudio/WCModeIntro.h \
  SvStudio/WCommand.h \
  SvStudio/WDebugTable.h \
  SvStudio/WMain.h \
  SvStudio/WNetFileList.h \
  SvStudio/WNetFileManager.h \
  SvStudio/WNetViewer.h \
  SvStudio/WOscillograph.h \
  SvStudio/WTextEditor.h \
  SvStudio/WTextSearchPanel.h \
  SvVMachine/Sv6Plc.h \
  SvVMachine/Sv7Sys.h \
  SvVMachine/SvVMachine.h \
  SvVMachine/SvVmByteCode.h \
  SvVMachine/SvVmCodeHeader.h \
  SvVMachine/SvVmTypes.h \
  SvVMachine/SvVmUtils.h \
  SvVMachine/SvVmVpu.h \
  SvVMachine/SvVmVpuState.h

SOURCES += \
  SvCompiler/SvClass.cpp \
  SvCompiler/SvCompilerConstExpression.cpp \
  SvCompiler/SvCompilerConstValue.cpp \
  SvCompiler/SvCompilerErrors.cpp \
  SvCompiler/SvCompilerExpression.cpp \
  SvCompiler/SvCompilerInit.cpp \
  SvCompiler/SvCompilerInputStream.cpp \
  SvCompiler/SvCompilerListing.cpp \
  SvCompiler/SvCompilerPreprocessor.cpp \
  SvCompiler/SvCompilerScaner.cpp \
  SvCompiler/SvCompilerSyntax.cpp \
  SvCompiler/SvFunction.cpp \
  SvCompiler/SvFunctionList.cpp \
  SvCompiler/SvFunctionType.cpp \
  SvCompiler/SvHelp.cpp \
  SvCompiler/SvOperator.cpp \
  SvCompiler/SvOperatorBlock.cpp \
  SvCompiler/SvOperatorReturn.cpp \
  SvCompiler/SvSource.cpp \
  SvCompiler/SvSourceFile.cpp \
  SvCompiler/SvType.cpp \
  SvCompiler/SvTypeList.cpp \
  SvCompiler/SvValue.cpp \
  SvCompiler/SvValueBinaryLong.cpp \
  SvCompiler/SvValueCall.cpp \
  SvCompiler/SvValueList.cpp \
  SvCompiler/SvValueMemberVariable.cpp \
  SvCompiler/SvValueVariable.cpp \
  SvCompiler/SvVariable.cpp \
  SvCompiler/SvVariableList.cpp \
  SvCompiler/SvVpuCompiler1.cpp \
  SvCompiler/SvVpuCompiler2.cpp \
  SvHost/SvDir.cpp \
  SvHost/SvMirror.cpp \
  SvHost/SvMirrorCom.cpp \
  SvHost/SvMirrorExtern.cpp \
  SvHost/SvMirrorLocal.cpp \
  SvHost/SvMirrorManager.cpp \
  SvHost/SvMirrorRemote.cpp \
  SvHost/SvProgramm.cpp \
  SvHost/SvTextStreamIn.cpp \
  SvHost/SvTextStreamOut.cpp \
  SvHost/SvVMachineLocal.cpp \
  SvNet/SvNetBlockInfo.cpp \
  SvNet/SvNetChannel.cpp \
  SvNet/SvNetHandler.cpp \
  SvNet/SvNetHandlerFile.cpp \
  SvNet/SvNetHandlerMirror.cpp \
  SvNet/SvNetServer.cpp \
  SvStudio/DNetProcess.cpp \
  SvStudio/DNetProcessFileDirRequest.cpp \
  SvStudio/DNetProcessFileOperation.cpp \
  SvStudio/DNetProcessFileRequest.cpp \
  SvStudio/DNetProcessFileSend.cpp \
  SvStudio/DPrjProp.cpp \
  SvStudio/DProcess.cpp \
  SvStudio/DTextEditorSettings.cpp \
  SvStudio/Highlighter.cpp \
  SvStudio/IngDebugCalculator.cpp \
  SvStudio/SvPeriodicParser.cpp \
  SvStudio/SvProject.cpp \
  SvStudio/SvUtils.cpp \
  SvStudio/WBrowser.cpp \
  SvStudio/WCModeBoard.cpp \
  SvStudio/WCModeEditor.cpp \
  SvStudio/WCModeHelp.cpp \
  SvStudio/WCModeIntro.cpp \
  SvStudio/WCommand.cpp \
  SvStudio/WDebugTable.cpp \
  SvStudio/WMain.cpp \
  SvStudio/WNetFileList.cpp \
  SvStudio/WNetFileManager.cpp \
  SvStudio/WNetViewer.cpp \
  SvStudio/WOscillograph.cpp \
  SvStudio/WTextEditor.cpp \
  SvStudio/WTextSearchPanel.cpp \
  SvStudio/bug.cpp \
  SvStudio/main.cpp \
  SvVMachine/SvVMachine.cpp


DISTFILES +=
