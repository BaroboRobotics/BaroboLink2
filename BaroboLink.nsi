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
!define INSTALLER_NAME "C:\Users\dko\Desktop\Output\BaroboLink\BaroboLink_setup.exe"
!define MAIN_APP_EXE "BaroboLink.exe"
!define FIRMUP_APP_EXE "MobotFirmwareUpdate.exe"
!define INSTALL_TYPE "SetShellVarContext all"
!define REG_ROOT "HKLM"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_APP_EXE}"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

!define REG_START_MENU "Start Menu Folder"

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
InstallDirRegKey "${REG_ROOT}" "${REG_APP_PATH}" ""
InstallDir "$PROGRAMFILES\BaroboLink"

######################################################################

!include "MUI.nsh"

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
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\BaroboLink.exe"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\freetype6.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\intl.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libatk-1.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libcairo-2.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libexpat-1.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libfontconfig-1.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgcc_s_dw2-1.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgdk-win32-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgdk_pixbuf-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgio-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libglib-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgmodule-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgobject-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgthread-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libgtk-win32-2.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libpango-1.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libpangocairo-1.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libpangoft2-1.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libpangowin32-1.0-0.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libpng14-14.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\libstdc++-6.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\MobotFirmwareUpdate.exe"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\pthreadGC2.dll"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\zlib1.dll"
SetOutPath "$INSTDIR\interface"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\16px_move_back.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\16px_move_back.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\16px_move_forward.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\16px_move_forward.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\16px_stop.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\16px_stop.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\DOF_joint_diagram.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\face_backward.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\iMobot.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\imobot_diagram.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\imobot_diagram.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\inch_left.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\inch_right.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\interface.glade"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\lface_forward.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\mobotfirmwareupdateinterface.glade"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\move_back.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\move_back.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\move_forward.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\move_forward.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\rotate_left.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\rotate_left.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\rotate_right.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\rotate_right.svg"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\stop.png"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\interface\stop.svg"
SetOutPath "$INSTDIR\hexfiles"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\hexfiles\linkbot_68.hex"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\hexfiles\linkbot_75.hex"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\hexfiles\linkbot_76.hex"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\hexfiles\rev3.hex"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\hexfiles\rev3_safe.hex"
File "C:\Users\dko\Projects\RoboMancer\BaroboLink_201305151230\hexfiles\rev4.hex"
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
Delete "$INSTDIR\MobotFirmwareUpdate.exe"
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

