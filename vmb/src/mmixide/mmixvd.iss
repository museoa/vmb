; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=mmixvd
AppVerName=mmixvd version 1.1, MMIX Visual Debugger
AppVersion=1.1
AppPublisher=Martin Ruckert
AppPublisherURL=http://www.cs.hm.edu/~ruckert/
AppSupportURL=http://mmix.cs.hm.edu
AppUpdatesURL=http://mmix.cs.hm.edu
DefaultDirName={pf}\mmix
DefaultGroupName=mmix
AllowNoIcons=yes
ChangesAssociations=yes
ChangesEnvironment=yes
LicenseFile=C:\home\mmixvd\mmixide\licensevd.txt
WizardImageFile="C:\home\mmixvd\mmixide\lsetupvd.bmp"
WizardImageBackColor=$127917
WizardImageStretch=no
WizardSmallImageFile="C:\home\mmixvd\mmixide\ssetupvd.bmp"

[Components]
Name: "mmix"; Description: "MMIX Command Line Tools"; Types: full compact custom;

[Tasks]
Name: helloapp; Description: "Put the hello world Application on your dektop"
Name: modifypath; Description: "Add application directory to your environment path"

[Files]
Source: "C:\home\mmixvd\mmixide\Release\mmixvd.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\home\mmixvd\mmixide\Release\mmix.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: mmix
Source: "C:\home\mmixvd\mmixide\Release\mmixal.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: mmix
Source: "C:\home\mmixvd\mmixide\Release\mmmix.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: mmix
Source: "C:\home\mmixvd\mmixide\Release\mmotype.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: mmix
Source: "C:\home\mmixvd\mmixide\Icons\mmixvd.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\home\mmixvd\mmixide\readmevd.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "C:\home\mmixvd\mmixide\licensevd.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\home\mmixvd\mmixide\copying.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\home\mmixvd\mmixide\mmixvd.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\home\mmixvd\mmixide\hello.mms"; DestDir: "{userdesktop}"; Flags: ignoreversion; Tasks: helloapp

[Icons]
;Name: "{userdesktop}\hello.mms"; Filename: "{app}\readme.vmb"; Tasks: desktopicon

[Run]
Filename: "{app}\mmixvd.exe"; Parameters: "{userdesktop}\hello.mms"; Description: "Launch MMIXVD with hello.mms"; Flags: nowait postinstall skipifsilent; Tasks: helloapp
Filename: "{app}\mmixvd.exe"; Parameters: ""; Description: "Launch MMIXVD"; Flags: nowait postinstall skipifsilent; Tasks: not helloapp

[Registry]
Root: HKCR; Subkey: ".mms"; ValueType: string; ValueName: ""; ValueData: "mms_auto_file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".mms"; ValueType: string; ValueName: "PerceivedType"; ValueData: "text"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "mms_auto_file"; ValueType: string; ValueName: ""; ValueData: "MMIX source"; Flags: uninsdeletekey
Root: HKCR; Subkey: "mms_auto_file\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\mmixvd.exe"" ""%1"""
Root: HKCR; Subkey: "mms_auto_file\shell\edit\command"; ValueType: string; ValueName: ""; ValueData: """{app}\mmixvd.exe"" ""%1"""

[Code]
const
    ModPathName = 'modifypath';
    ModPathType = 'system';

function ModPathDir(): TArrayOfString;
begin
    setArrayLength(Result, 1)
    Result[0] := ExpandConstant('{app}');
end;
#include "modpath.iss"

