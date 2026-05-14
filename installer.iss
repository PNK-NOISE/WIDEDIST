[Setup]
AppName=WIDEDIST
AppVersion=1.0.0
AppPublisher=PNK NOISE
DefaultDirName={commoncf64}\VST3\WIDEDIST.vst3
DisableDirPage=yes
DefaultGroupName=WIDEDIST
DisableProgramGroupPage=yes
OutputBaseFilename=WIDEDIST_Windows_Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "build\WIDEDIST_artefacts\Release\VST3\WIDEDIST.vst3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Note: JUCE VST3 builds as a folder with .vst3 extension. We copy its contents to {app} which is the .vst3 folder.

[Messages]
FinishedHeadingLabel=Setup has finished installing WIDEDIST
FinishedLabel=WIDEDIST has been installed in your VST3 folder.%n%nPlease rescan your plugins in your DAW to use it.
