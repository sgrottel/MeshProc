#
# Native to Managed project generation
#

# Fetch native project's packages
$packages = Select-Xml -Path (Join-Path $PSScriptRoot "../../MeshProc/packages.config") -XPath "/packages/package" `
    | Select-Object -ExpandProperty Node `
    | Foreach-Object { @{id=$_.id;version=$_.version} }

$proxyProj = Join-Path $PSScriptRoot "MeshProc.csproj"

if (Test-Path $proxyProj)
{
    $proxyPackages = Select-Xml -Path $proxyProj -XPath "/Project/ItemGroup/PackageReference" `
        | Select-Object -ExpandProperty Node `
        | Foreach-Object { @{id=$_.Include;version=$_.Version} }

    $norm = {
        param($h)
        "$($h.id)|$($h.version)"
    }

    $setA = $packages      | ForEach-Object $norm | Sort-Object
    $setB = $proxyPackages | ForEach-Object $norm | Sort-Object

    $equal = ($setA -join "`n") -eq ($setB -join "`n")

    if ($equal) {
        Write-Output "No change in nuget package references"
        exit 0
    }
}

# Generate managed proxy project
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
