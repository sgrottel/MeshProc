#
# Restores dependencies pre build
#
Write-Host "Restoring dependencies"

# Nuget
Write-Host "Nuget" -ForegroundColor Cyan -BackgroundColor Black

nuget restore (Join-Path $PSScriptRoot "packages.config") -PackagesDirectory (Join-Path $PSScriptRoot "packages")

# vcpkg
Write-Host "vcpkg" -ForegroundColor Cyan -BackgroundColor Black

if (-not (Test-Path (Join-Path $PSScriptRoot "vcpkg\vcpkg.exe") -PathType Leaf))
{
    .\vcpkg\bootstrap-vcpkg.bat
}

.\vcpkg\vcpkg.exe install --triplet x64-windows "--vcpkg-root=$(Join-Path $PSScriptRoot 'vcpkg')"

# done.
Write-Host "done."
