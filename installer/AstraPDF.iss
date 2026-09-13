#define MyAppName "AstraPDF"
#define MyAppVersion "0.2.0"
#define MyAppPublisher "AstraPDF"
#define MyAppExeName "AstraPDF.exe"

[Setup]
AppId={{8E7A9A56-BE73-4A7E-B6F2-33BEACF7A2B1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\AstraPDF
DefaultGroupName=AstraPDF
AllowNoIcons=yes
OutputDir=..\installer-output
OutputBaseFilename=AstraPDF-Setup-Windows-x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
Source: "..\dist\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\AstraPDF"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\AstraPDF"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch AstraPDF"; Flags: nowait postinstall skipifsilent
