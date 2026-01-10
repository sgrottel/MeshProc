#
# Managed to Native project update
#

$packagesConfig = Join-Path $PSScriptRoot "../../MeshProc/packages.config"

$proxyPackages = Select-Xml -Path (Join-Path $PSScriptRoot "MeshProc.csproj") -XPath "/Project/ItemGroup/PackageReference" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { @{id=$_.Include;version=$_.Version} }

$nativePackages = Select-Xml -Path $packagesConfig -XPath "/packages/package" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { @{id=$_.id;version=$_.version} }

$proxyPackages | Foreach-Object {
    $n = $nativePackages | Where-Object "id" -eq ($_.id)
    if ($n -eq $null) { throw "package $($_.id) not found!" }
    if ($n -is [object[]]) {
        if ($n.Count -eq 1) {
            $n = $n[0]
        } else { throw "Multiple packages matches $($_.id)" }
    }
    if ($_.version -eq $n.version) { return }

    Write-Host "Updating $($_.id) from $($n.version) to $($_.version)"

    nuget update $packagesConfig -Id ($_.id) -Version ($_.version)
}
