#
# Native to Managed project generation
#

# This is called as custom build step on the native project's `packages.config`

# Fetch native project's packages
$packages = Select-Xml -Path (Join-Path $PSScriptRoot "../../MeshProc/packages.config") -XPath "/packages/package" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { @{id=$_.id;version=$_.version} }

$proxyProj = Join-Path $PSScriptRoot "MeshProc.csproj"
$writeProxyProj = $true

if (Test-Path $proxyProj)
{
    $proxyPackages = @{}
    Select-Xml -Path $proxyProj -XPath "/Project/ItemGroup/PackageReference" `
        | Select-Object -ExpandProperty Node `
        | Foreach-Object { $proxyPackages[$_.Include] = $_.Version }

    $writeProxyProj = $false

    $packages | Foreach-Object {
        if ($proxyPackages.ContainsKey($_.id))
        {
            $proxyVersion = [Version]$proxyPackages[$_.id]
            $thisVersion = [Version]$_.version
            if ($proxyVersion -eq $thisVersion)
            {
                return
            }
            elseif ($proxyVersion -lt $thisVersion)
            {
                $writeProxyProj = $true
                Write-Host "> Updated package $($_.id) from $proxyVersion to $thisVersion"
            }
            else
            {
                throw ("Package $($_.id) is newer in proxy project: $proxyVersion >= $thisVersion "`
                    + "You must run `m2n.ps1` to consolidate project configurations before you can build the project.")
            }
        }
        else
        {
            $writeProxyProj = $true
            Write-Host "> New package $($_.id) at $($_.version)"
        }
    }
}

Write-Host "> writeProxyProj = $writeProxyProj"
if (-not $writeProxyProj)
{
    exit 0
}

Write-Host "> Generate managed proxy project" 
$xml = New-Object -TypeName System.Xml.XmlDocument
$root = $xml.AppendChild($xml.CreateElement("Project"))
$root.SetAttribute("Sdk", "Microsoft.NET.Sdk")
$root.AppendChild($xml.CreateElement("PropertyGroup")).AppendChild($xml.CreateElement("TargetFramework")).InnerText = "native"
$items = $root.AppendChild($xml.CreateElement("ItemGroup"))
$packages | ForEach-Object {
    $item = $items.AppendChild($xml.CreateElement("PackageReference"));
    $item.SetAttribute("Include", $_.id);
    $item.SetAttribute("Version", $_.version);
}
$xml.Save($proxyProj)
