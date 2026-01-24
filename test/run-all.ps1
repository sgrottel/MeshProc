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

Write-Host
Write-Host "run-test-obj.ps1" -Foreground Cyan -Background Black
Write-Host
& (Join-Path $PSScriptRoot "run-test-obj.ps1") -exe:$exe @innerParams

Write-Host
Write-Host "run-test-commands-1.ps1" -Foreground Cyan -Background Black
Write-Host
& (Join-Path $PSScriptRoot "run-test-commands-1.ps1") -exe:$exe @innerParams

Write-Host
Write-Host "run-test-selection.ps1" -Foreground Cyan -Background Black
Write-Host
& (Join-Path $PSScriptRoot "run-test-selection.ps1") -exe:$exe @innerParams

Write-Host
Write-Host "done." -Foreground Cyan -Background Black
Write-Host
