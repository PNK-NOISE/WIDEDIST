[Setup]
AppName=DISTWIDE
AppVersion=1.0.0
AppPublisher=PNK NOISE
DefaultDirName={commoncf64}\VST3\DISTWIDE.vst3
DisableDirPage=yes
DefaultGroupName=DISTWIDE
DisableProgramGroupPage=yes
OutputBaseFilename=DISTWIDE_Windows_Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "build\TapeDist_artefacts\Release\VST3\TapeDist.vst3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Note: JUCE VST3 builds as a folder with .vst3 extension. We copy its contents to {app} which is the .vst3 folder.

[Messages]
FinishedHeadingLabel=Setup has finished installing DISTWIDE
FinishedLabel=DISTWIDE has been installed in your VST3 folder.%n%nPlease rescan your plugins in your DAW to use it.
