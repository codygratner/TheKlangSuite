Write-Host "Waiting for MSBuild to finish..."
while (Get-Process -Name msbuild -ErrorAction SilentlyContinue) {
    Start-Sleep -Seconds 1
}

Write-Host "Copying VST3s..."
Copy-Item -Path "build/TheKlangFarmer_artefacts/Release/VST3/The Klang Farmer.vst3" -Destination "C:\Program Files\Common Files\VST3\" -Recurse -Force
Copy-Item -Path "build/TheKlangPlanter_artefacts/Release/VST3/The Klang Planter.vst3" -Destination "C:\Program Files\Common Files\VST3\" -Recurse -Force
Copy-Item -Path "build/FormlessSpawn_artefacts/Release/VST3/Formless Spawn.vst3" -Destination "C:\Program Files\Common Files\VST3\" -Recurse -Force
Copy-Item -Path "build/DigiStrikeFS2_artefacts/Release/VST3/DigiStrike FS2.vst3" -Destination "C:\Program Files\Common Files\VST3\" -Recurse -Force
Copy-Item -Path "build/TheKlangFarmer_artefacts/Release/VST3/The Klang Farmer.vst3" -Destination "current_build/VST3/" -Recurse -Force
Copy-Item -Path "build/TheKlangPlanter_artefacts/Release/VST3/The Klang Planter.vst3" -Destination "current_build/VST3/" -Recurse -Force
Copy-Item -Path "build/FormlessSpawn_artefacts/Release/VST3/Formless Spawn.vst3" -Destination "current_build/VST3/" -Recurse -Force
Copy-Item -Path "build/DigiStrikeFS2_artefacts/Release/VST3/DigiStrike FS2.vst3" -Destination "current_build/VST3/" -Recurse -Force

Write-Host "Copying Standalones..."
Copy-Item -Path "build/TheKlangFarmer_artefacts/Release/Standalone/The Klang Farmer.exe" -Destination "current_build/Standalone/" -Force
Copy-Item -Path "build/TheKlangPlanter_artefacts/Release/Standalone/The Klang Planter.exe" -Destination "current_build/Standalone/" -Force
Copy-Item -Path "build/FormlessSpawn_artefacts/Release/Standalone/Formless Spawn.exe" -Destination "current_build/Standalone/" -Force
Copy-Item -Path "build/DigiStrikeFS2_artefacts/Release/Standalone/DigiStrike FS2.exe" -Destination "current_build/Standalone/" -Force

Write-Host "Copying Editor..."
Copy-Item -Path "build/TheKlangEditor_artefacts/Release/The Klang Editor.exe" -Destination "current_build/Editor/" -Force

Write-Host "Done!"
