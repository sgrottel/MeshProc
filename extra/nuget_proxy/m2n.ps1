#
# Managed to Native project update
#

# Only supports project upgrades!

$packagesConfig = Join-Path $PSScriptRoot "../../MeshProc/packages.config"

$proxyPackages = Select-Xml -Path (Join-Path $PSScriptRoot "MeshProc.csproj") -XPath "/Project/ItemGroup/PackageReference" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { @{id=$_.Include;version=$_.Version} }

$nativePackages = @{}
Select-Xml -Path $packagesConfig -XPath "/packages/package" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { $nativePackages[$_.id] = $_.version }

$proxyPackages | Foreach-Object {
    if ($nativePackages.ContainsKey($_.id))
    {
        $nativeVersion = [Version]$nativePackages[$_.id]
        $proxyVersion = [Version]$_.version

        if ($nativeVersion -eq $proxyVersion)
        {
            # package ok
        }
        elseif ($nativeVersion -lt $proxyVersion)
        {
            Write-Host "Upgrading package $($_.id) from $nativeVersion to $proxyVersion"
            nuget update $packagesConfig -Id ($_.id) -Version ($proxyVersion)
        }
        else
        {
            throw ("Automatically downgrading package $($_.id) from $nativeVersion to $proxyVersion is not possible.")
        }
    }
    else
    {
        Write-Host "> Proxy package $($_.id) not found in native project"
    }
}
