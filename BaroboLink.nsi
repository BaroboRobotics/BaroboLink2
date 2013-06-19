############################################################################################
#      NSIS Installation Script created by NSIS Quick Setup Script Generator v1.09.18
#               Entirely Edited with NullSoft Scriptable Installation System                
#              by Vlasis K. Barkas aka Red Wine red_wine@freemail.gr Sep 2006               
############################################################################################

!define APP_NAME "BaroboLink"
!define FIRMUP_APP_NAME "Barobo Firmware Utility"
!define COMP_NAME "Barobo"
!define WEB_SITE "http://www.barobo.com"
!define VERSION "00.02.00.00"
!define COPYRIGHT "Barobo  © 2013"
!define DESCRIPTION "Application"
!define INSTALLER_NAME "BaroboLink_setup.exe"
!define MAIN_APP_EXE "BaroboLink.exe"
!define FIRMUP_APP_EXE "BaroboFirmwareUpdate.exe"
!define INSTALL_TYPE "SetShellVarContext all"
!define REG_ROOT "HKLM"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_APP_EXE}"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

!define REG_START_MENU "Start Menu Folder"

var SM_Folder
var chhome
var OUT

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
InstallDirRegKey "${REG_ROOT}" "${REG_APP_PATH}" ""
InstallDir "$PROGRAMFILES\BaroboLink"

######################################################################

!include "MUI.nsh"
!include "LogicLib.nsh"
!include "NSISpcre.nsh"

!insertmacro REReplace

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING

!insertmacro MUI_PAGE_WELCOME

!ifdef LICENSE_TXT
!insertmacro MUI_PAGE_LICENSE "${LICENSE_TXT}"
!endif

!insertmacro MUI_PAGE_DIRECTORY

!ifdef REG_START_MENU
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "BaroboLink"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${REG_ROOT}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINSTALL_PATH}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${REG_START_MENU}"
!insertmacro MUI_PAGE_STARTMENU Application $SM_Folder
!endif

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${MAIN_APP_EXE}"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

######################################################################

Section -MainProgram
${INSTALL_TYPE}
SetOverwrite ifnewer
SetOutPath "$INSTDIR"
File "BaroboLink.exe"
File "freetype6.dll"
File "intl.dll"
File "libatk-1.0-0.dll"
File "libcairo-2.dll"
File "libexpat-1.dll"
File "libfontconfig-1.dll"
File "libgcc_s_dw2-1.dll"
File "libgdk-win32-2.0-0.dll"
File "libgdk_pixbuf-2.0-0.dll"
File "libgio-2.0-0.dll"
File "libglib-2.0-0.dll"
File "libgmodule-2.0-0.dll"
File "libgobject-2.0-0.dll"
File "libgthread-2.0-0.dll"
File "libgtk-win32-2.0-0.dll"
File "libpango-1.0-0.dll"
File "libpangocairo-1.0-0.dll"
File "libpangoft2-1.0-0.dll"
File "libpangowin32-1.0-0.dll"
File "libpng14-14.dll"
File "libstdc++-6.dll"
File "BaroboFirmwareUpdate.exe"
File "pthreadGC2.dll"
File "zlib1.dll"
File "Barobo_Linkbot_Driver.exe"
SetOutPath "$INSTDIR\interface"
File "interface\16px_move_back.png"
File "interface\16px_move_back.svg"
File "interface\16px_move_forward.png"
File "interface\16px_move_forward.svg"
File "interface\16px_stop.png"
File "interface\16px_stop.svg"
File "interface\DOF_joint_diagram.png"
File "interface\face_backward.svg"
File "interface\iMobot.png"
File "interface\imobot_diagram.png"
File "interface\imobot_diagram.svg"
File "interface\inch_left.svg"
File "interface\inch_right.svg"
File "interface\interface.glade"
File "interface\lface_forward.svg"
File "interface\mobotfirmwareupdateinterface.glade"
File "interface\move_back.png"
File "interface\move_back.svg"
File "interface\move_forward.png"
File "interface\move_forward.svg"
File "interface\rotate_left.png"
File "interface\rotate_left.svg"
File "interface\rotate_right.png"
File "interface\rotate_right.svg"
File "interface\stop.png"
File "interface\stop.svg"
SetOutPath "$INSTDIR\docs"
File "libbarobo\docs\barobo.pdf"
File "libbarobo\docs\index.html"
File "libbarobo\docs\Barobo.png"
SetOutPath "$INSTDIR\hexfiles"
File "hexfiles\linkbot_68.hex"
File "hexfiles\linkbot_75.hex"
File "hexfiles\linkbot_76.hex"
File "hexfiles\rev3.hex"
File "hexfiles\rev3_safe.hex"
File "hexfiles\rev4.hex"

# Install the Ch package
# First, figure out if Ch in installed already.
ReadRegStr $chhome HKLM SOFTWARE\SoftIntegration "CHHOME"

# Change slashes to backslashes
${REReplace} $OUT "\/" $chhome "\\" 1

${If} $OUT == ""
  StrCpy $OUT "C:\Ch"
${Else}
  RMDir /r "$OUT\package\chbarobo"
  Delete "$OUT\toolkit\include\mobot.h"
  Delete "$OUT\toolkit\include\linkbot.h"
${EndIf}
SetOutPath "$OUT\package"
File /r "..\chmobot\chbarobo"

GetVersion::WindowsPlatformArchitecture
Pop $R0
DetailPrint $R0
${If} $R0 == "64"
Rename "$OUT\package\chbarobo\dl\Win64\libmobot.dl" "$OUT\package\chbarobo\dl\libmobot.dl" 
Rename "$OUT\package\chbarobo\dl\Win64\Microsoft.VC80.CRT" "$OUT\package\chbarobo\dl\Microsoft.VC80.CRT" 
${Else}
Rename "$OUT\package\chbarobo\dl\Windows\libmobot.dl" "$OUT\package\chbarobo\dl\libmobot.dl" 
Rename "$OUT\package\chbarobo\dl\Windows\Microsoft.VC90.CRT" "$OUT\package\chbarobo\dl\Microsoft.VC90.CRT" 
${Endif}

# Copy chbarobo header files to toolkit/include directory
${If} $chhome != ""
CopyFiles $OUT\package\chbarobo\include\mobot.h $OUT\toolkit\include\mobot.h
CopyFiles $OUT\package\chbarobo\include\linkbot.h $OUT\toolkit\include\linkbot.h
${Endif}

SectionEnd

######################################################################

Section -Icons_Reg
SetOutPath "$INSTDIR"
WriteUninstaller "$INSTDIR\uninstall.exe"

!ifdef REG_START_MENU 
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
CreateDirectory "$SMPROGRAMS\$SM_Folder"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${FIRMUP_APP_NAME}.lnk" "$INSTDIR\${FIRMUP_APP_EXE}"
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!insertmacro MUI_STARTMENU_WRITE_END
!endif

!ifndef REG_START_MENU
CreateDirectory "$SMPROGRAMS\BaroboLink"
CreateShortCut "$SMPROGRAMS\BaroboLink\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\BaroboLink\${FIRMUP_APP_NAME}.lnk" "$INSTDIR\${FIRMUP_APP_EXE}"
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\BaroboLink\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\BaroboLink\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!endif

WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "PATH" "$INSTDIR"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayName" "${APP_NAME}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayIcon" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayVersion" "${VERSION}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "Publisher" "${COMP_NAME}"

!ifdef WEB_SITE
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "URLInfoAbout" "${WEB_SITE}"
!endif
SectionEnd

######################################################################

Section Uninstall
${INSTALL_TYPE}
Delete "$INSTDIR\BaroboLink.exe"
Delete "$INSTDIR\freetype6.dll"
Delete "$INSTDIR\intl.dll"
Delete "$INSTDIR\libatk-1.0-0.dll"
Delete "$INSTDIR\libcairo-2.dll"
Delete "$INSTDIR\libexpat-1.dll"
Delete "$INSTDIR\libfontconfig-1.dll"
Delete "$INSTDIR\libgcc_s_dw2-1.dll"
Delete "$INSTDIR\libgdk-win32-2.0-0.dll"
Delete "$INSTDIR\libgdk_pixbuf-2.0-0.dll"
Delete "$INSTDIR\libgio-2.0-0.dll"
Delete "$INSTDIR\libglib-2.0-0.dll"
Delete "$INSTDIR\libgmodule-2.0-0.dll"
Delete "$INSTDIR\libgobject-2.0-0.dll"
Delete "$INSTDIR\libgthread-2.0-0.dll"
Delete "$INSTDIR\libgtk-win32-2.0-0.dll"
Delete "$INSTDIR\libpango-1.0-0.dll"
Delete "$INSTDIR\libpangocairo-1.0-0.dll"
Delete "$INSTDIR\libpangoft2-1.0-0.dll"
Delete "$INSTDIR\libpangowin32-1.0-0.dll"
Delete "$INSTDIR\libpng14-14.dll"
Delete "$INSTDIR\libstdc++-6.dll"
Delete "$INSTDIR\BaroboFirmwareUpdate.exe"
Delete "$INSTDIR\pthreadGC2.dll"
Delete "$INSTDIR\zlib1.dll"
Delete "$INSTDIR\interface\16px_move_back.png"
Delete "$INSTDIR\interface\16px_move_back.svg"
Delete "$INSTDIR\interface\16px_move_forward.png"
Delete "$INSTDIR\interface\16px_move_forward.svg"
Delete "$INSTDIR\interface\16px_stop.png"
Delete "$INSTDIR\interface\16px_stop.svg"
Delete "$INSTDIR\interface\DOF_joint_diagram.png"
Delete "$INSTDIR\interface\face_backward.svg"
Delete "$INSTDIR\interface\iMobot.png"
Delete "$INSTDIR\interface\imobot_diagram.png"
Delete "$INSTDIR\interface\imobot_diagram.svg"
Delete "$INSTDIR\interface\inch_left.svg"
Delete "$INSTDIR\interface\inch_right.svg"
Delete "$INSTDIR\interface\interface.glade"
Delete "$INSTDIR\interface\lface_forward.svg"
Delete "$INSTDIR\interface\mobotfirmwareupdateinterface.glade"
Delete "$INSTDIR\interface\move_back.png"
Delete "$INSTDIR\interface\move_back.svg"
Delete "$INSTDIR\interface\move_forward.png"
Delete "$INSTDIR\interface\move_forward.svg"
Delete "$INSTDIR\interface\rotate_left.png"
Delete "$INSTDIR\interface\rotate_left.svg"
Delete "$INSTDIR\interface\rotate_right.png"
Delete "$INSTDIR\interface\rotate_right.svg"
Delete "$INSTDIR\interface\stop.png"
Delete "$INSTDIR\interface\stop.svg"
Delete "$INSTDIR\hexfiles\linkbot_68.hex"
Delete "$INSTDIR\hexfiles\linkbot_75.hex"
Delete "$INSTDIR\hexfiles\linkbot_76.hex"
Delete "$INSTDIR\hexfiles\rev3.hex"
Delete "$INSTDIR\hexfiles\rev3_safe.hex"
Delete "$INSTDIR\hexfiles\rev4.hex"
 
RmDir "$INSTDIR\hexfiles"
RmDir "$INSTDIR\interface"
 
Delete "$INSTDIR\uninstall.exe"
!ifdef WEB_SITE
Delete "$INSTDIR\${APP_NAME} website.url"
!endif

RmDir "$INSTDIR"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_GETFOLDER "Application" $SM_Folder
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\$SM_Folder\${FIRMUP_APP_NAME}.lnk"
Delete "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\$SM_Folder"
!endif

!ifndef REG_START_MENU
Delete "$SMPROGRAMS\BaroboLink\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\BaroboLink\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\BaroboLink\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\BaroboLink"
!endif

DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"
SectionEnd

######################################################################

