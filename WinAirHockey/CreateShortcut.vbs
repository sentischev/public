Set idWS = WScript.CreateObject("WScript.Shell")
Set idLink = idWS.CreateShortcut("WinAirHockey.lnk")
Set idFso = WScript.CreateObject("Scripting.FileSystemObject")
idLink.TargetPath = idFso.GetAbsolutePathName(".") & "\\" & "WinAirHockey.exe"
idLink.Save
