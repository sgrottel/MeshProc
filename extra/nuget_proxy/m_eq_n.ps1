#
# Ensures that Managed to Native projects use the same packages
#

$proxyPackages = Select-Xml -Path (Join-Path $PSScriptRoot "MeshProc.csproj") -XPath "/Project/ItemGroup/PackageReference" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { @{id=$_.Include;version=$_.Version} }

$nativePackages = Select-Xml -Path (Join-Path $PSScriptRoot "../../MeshProc/packages.config") -XPath "/packages/package" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { @{id=$_.id;version=$_.version} }

$setA = $nativePackages | ForEach-Object { "$($_.id)|$($_.version)" } | Sort-Object
$setB = $proxyPackages  | ForEach-Object { "$($_.id)|$($_.version)" } | Sort-Object

$strA = $setA -join "`n"
$strB = $setB -join "`n"

Write-Host "Packages from native project:`n$strA"
Write-Host "Packages from managed proxy project:`n$strB"

$equal = $strA -eq $strB

if ($equal) {
    Write-Output "Package references are in sync"
    exit 0
} else {
    Write-Error "Package references are out of sync"
    exit 1
}
