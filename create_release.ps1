$date = (Get-Date).ToString("yyyyMMdd")
cmd /C "C:\Program Files\GitHub CLI\gh.exe" release create "v$date-beta" ..\..\plugin-GUI\Build\Release\plugins\OpenEphysNeuroOmega.dll