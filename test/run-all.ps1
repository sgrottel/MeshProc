[CmdletBinding()]
param(
	[Parameter(Mandatory = $true)][string]$exe
)

# Forward common parameters if present
$innerParams = @{
	exe = $exe
}
foreach ($key in 'Verbose','Debug','ErrorAction','WarningAction') {
	if ($PSBoundParameters.ContainsKey($key)) {
		$innerParams[$key] = $PSBoundParameters[$key]
	}
}

$tests = Get-ChildItem -Path (Join-Path $PSScriptRoot run*.ps1) -exclude run-all.ps1

$tests | foreach-object {
	Write-Host
	Write-Host $_.Name -Foreground Cyan -Background Black
	Write-Host
	& $_.FullName -exe:$exe @innerParams
}

Write-Host
Write-Host "done." -Foreground Cyan -Background Black
Write-Host
