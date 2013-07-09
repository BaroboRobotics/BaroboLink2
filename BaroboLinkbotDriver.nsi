############################################################################################
#      NSIS Installation Script created by NSIS Quick Setup Script Generator v1.09.18
#               Entirely Edited with NullSoft Scriptable Installation System                
#              by Vlasis K. Barkas aka Red Wine red_wine@freemail.gr Sep 2006               
############################################################################################

!define APP_NAME "Barobo Linkbot Driver"
!define COMP_NAME "Barobo"
!define WEB_SITE "http://www.barobo.com"
!define COPYRIGHT "Barobo  © 2013"
!define DESCRIPTION "Driver"
!define INSTALLER_NAME "Barobo_Linkbot_Driver_Win7.exe"
!define INSTALL_TYPE "SetShellVarContext all"
!define VERSION "00.00.01.00"

!finalize '"C:/Program Files/Microsoft SDKs/Windows/v7.0A/bin/signtool.exe" sign "${INSTALLER_NAME}"'

var SM_Folder

######################################################################

VIProductVersion  "${VERSION}"
VIAddVersionKey "ProductName"  "${APP_NAME}"
VIAddVersionKey "CompanyName"  "${COMP_NAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "FileDescription"  "${DESCRIPTION}"
VIAddVersionKey "FileVersion"  "${VERSION}"

######################################################################

SetCompressor ZLIB
Name "${APP_NAME}"
Caption "${APP_NAME}"
OutFile "${INSTALLER_NAME}"
BrandingText "${APP_NAME}"
XPStyle on
RequestExecutionLevel admin

######################################################################

!include "MUI.nsh"
!include "LogicLib.nsh"

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING

!insertmacro MUI_PAGE_WELCOME

!ifdef LICENSE_TXT
!insertmacro MUI_PAGE_LICENSE "${LICENSE_TXT}"
!endif

#!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

######################################################################

Section "Install Linkbot Driver" SectionDriver
    SetOutPath $TEMP
    File "Barobo_Linkbot_Driver.inf"
    File "barobo_linkbot_driver.cat"
    File "Barobo_Linkbot_Driver\dpinst_x86.exe"
    File "Barobo_Linkbot_Driver\dpinst_x64.exe"
#ExecWait '$SYSDIR\pnputil.exe -f -a "$TEMP\Barobo_Linkbot_Driver.inf" > C:\Users\dko\junk1 2>&1' $1
#ExecWait '$SYSDIR\cmd /K $SYSDIR\pnputil.exe -f -a "$TEMP\Barobo_Linkbot_Driver.inf"' 
#ExecWait 'C:\Windows\SysWOW64\cmd.exe /K $SYSDIR\pnputil.exe -f -a "$TEMP\Barobo_Linkbot_Driver.inf"' 
    GetVersion::WindowsPlatformArchitecture
    Pop $R0
    ${If} $R0 == "64"
      ExecWait '"$TEMP\dpinst_x64.exe"'
    ${Else}
      ExecWait '"$TEMP\dpinst_x86.exe"'
    ${Endif}
#ExecWait '"$TEMP\installdriver.bat" > C:\Users\dko\junk2 2>&1' $2
#    messageBox MB_OK "batfile returned $2"
#    ExecShell "" '$SYSDIR\pnputil.exe -f -a $TEMP\Barobo_Linkbot_Driver.inf' 

SectionEnd

######################################################################

# Component/Section Descriptions
LangString DESC_SectionDriver ${LANG_ENGLISH} "The Linkbot Driver: Required for controlling a Linkbot with a Windows computer"
LangString DESC_SectionPython ${LANG_ENGLISH} "Python: A scripting language for running Linkbot control programs"
LangString DESC_SectionPyBarobo ${LANG_ENGLISH} "The Barobo Python module used to control Linkbots with Python"
LangString DESC_SectionMatplotlib ${LANG_ENGLISH} "A scientific and plotting module used to generate 2-D plots in Python programs"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionDriver} $(DESC_SectionDriver)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionPython} $(DESC_SectionPython)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionPyBarobo} $(DESC_SectionPyBarobo)
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionMatplotlib} $(DESC_SectionMatplotlib)
!insertmacro MUI_FUNCTION_DESCRIPTION_END


######################################################################
 
