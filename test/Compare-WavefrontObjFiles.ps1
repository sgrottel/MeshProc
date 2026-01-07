param(
	[Parameter(Mandatory = $true)][string]$FileA,
	[Parameter(Mandatory = $true)][string]$FileB
)

function Equals-Epsilon {
	param(
		[float]$a,
		[float]$b,
		[float]$eps = 1e-6
	)
	return [math]::Abs($a - $b) -le $eps
}

function Find-Vec3Index {
	param(
		[Collections.Generic.List[ValueTuple[float,float,float]]]$List,
		[float]$x,
		[float]$y,
		[float]$z,
		[float]$eps = 1e-6
	)

	for ($i = 0; $i -lt $List.Count; $i++) {
		$v = $List[$i]

		if ( (Equals-Epsilon $v.Item1 $x $eps) -and
				(Equals-Epsilon $v.Item2 $y $eps) -and
				(Equals-Epsilon $v.Item3 $z $eps) ) {
			return $i
		}
	}

	return -1   # not found
}

function Load-WavefrontObjFile
{
	param(
		[Parameter(Mandatory = $true)][string]$File
	)
	Write-Verbose "Loading $File"

	function GetOrAdd-Vec3 {
		param(
			[Collections.Generic.List[ValueTuple[float,float,float]]]$List,
			[float]$x,
			[float]$y,
			[float]$z,
			[float]$eps = 1e-6
		)

		$idx = Find-Vec3Index -List $List -x $x -y $y -z $z -eps $eps
		if ($idx -ge 0) {
			return $idx
		}

		# Add new entry
		$List.Add([ValueTuple[float,float,float]]::new($x, $y, $z))
		return $List.Count - 1
	}

	$lines = [string[]](Get-Content -Path $File)

	$nextVIdx = 1;

	$mapV = @{}
	$verts = [Collections.Generic.List[ValueTuple[float,float,float]]]::new()
	$tris = [Collections.Generic.List[ValueTuple[int,int,int]]]::new()

	$lines | foreach {
		$l = $_.Trim()

		if ($l.StartsWith('v ')) {
			if ($l -match '^v\s+(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)') {
				$x = [float]$Matches[1]
				$y = [float]$Matches[2]
				$z = [float]$Matches[3]

				$idx = GetOrAdd-Vec3 -List $verts -x $x -y $y -z $z
				$mapV[$mapV.Count + 1] = $idx
			} else {
				throw ("Failed to parse 'f' line: " + $_)
			}
		}
		elseif ($l.StartsWith('f ')) {
			if ($l -match '^f\s+(\d+)[^\s]*\s+(\d+)[^\s]*\s+(\d+)') {
				$a = [int]$Matches[1]
				$b = [int]$Matches[2]
				$c = [int]$Matches[3]

				if (($a -eq $b) -or ($a -eq $c)) { throw "Degenerated triangle" }

				$tris.Add([ValueTuple[int,int,int]]::new($a, $b, $c))
			} else {
				throw ("Failed to parse 'f' line: " + $_)
			}
		}
	}

	for ($i = 0; $i -lt $tris.Count; $i++) {
		$t = $tris[$i]

		if (-not $mapV.ContainsKey($t.Item1)) { throw "Missing key: $($t.Item1)" }
		if (-not $mapV.ContainsKey($t.Item2)) { throw "Missing key: $($t.Item2)" }
		if (-not $mapV.ContainsKey($t.Item3)) { throw "Missing key: $($t.Item3)" }

		if (($mapV[$t.Item1] -eq $mapV[$t.Item2]) -or ($mapV[$t.Item1] -eq $mapV[$t.Item3])) { throw "Degenerated triangle" }

		$tris[$i] = [ValueTuple[int,int,int]]::new(
			$mapV[$t.Item1],
			$mapV[$t.Item2],
			$mapV[$t.Item3]
		)
	}

	Write-Verbose "$($verts.Count) vertices, from $($mapV.Count) v entries"
	Write-Verbose "$($tris.Count) triangles"

	return [pscustomobject]@{ verts = $verts; tris = $tris }
}

function CompareVertices
{
	param(
		[pscustomobject]$meshA,
		[pscustomobject]$meshB
	)
	# throws if not equal
	# Returns a map from vertex indices in meshB onto vertex indices in meshA

	if ($meshA.verts.Count -ne $meshB.verts.Count) { throw "Different number of unique vertices" }

	$mapB2A = @{}

	$useA = [System.Collections.Generic.HashSet[int]]::new()

	for ($i = 0; $i -lt $meshB.verts.Count; $i++) {
		$idx = Find-Vec3Index -List $meshA.verts -x $meshB.verts[$i].Item1 -y $meshB.verts[$i].Item2 -z $meshB.verts[$i].Item3
		if ($idx -lt 0) { throw "Vertex from B not found in A: $($meshB.verts[$i])" }
		$mapB2A[$i] = $idx
		if (-not $useA.Add($idx)) { throw "Duplicated entry in list of unique values" }
	}

	for ($i = 0; $i -lt $meshA.verts.Count; $i++) {
		if (-not $useA.Contains($i)) { throw "Vertex $i in A is not referenced from B: $($meshA.verts[$i])" }
	}

	# remap tris in B to use vertices from A
	for ($i = 0; $i -lt $meshB.tris.Count; $i++) {
		$t = $meshB.tris[$i]
		$meshB.tris[$i] = [ValueTuple[int,int,int]]::new(
			$mapB2A[$t.Item1],
			$mapB2A[$t.Item2],
			$mapB2A[$t.Item3]
		)
	}

	$meshB.verts = $null
}

function CompareAndRemoveTriangles
{
	param(
		[pscustomobject]$meshA,
		[pscustomobject]$meshB
	)
	# Removes all triangles from A and B which are present in both

	if ($meshA.tris.Count -ne $meshB.tris.Count) { throw "Different number of unique triangles" }

	function Orient-Triangles {
		param(
			[Collections.Generic.List[ValueTuple[int, int, int]]]$tris
		)

		for ($i = 0; $i -lt $tris.Count; $i++) {
			$t = $tris[$i]
			if (($t.Item1 -lt $t.Item2) -and ($t.Item3 -lt $t.Item2))
			{
				$t = [ValueTuple[int,int,int]]::new(
					$t.Item2,
					$t.Item3,
					$t.Item1
				)
			}
			if (($t.Item1 -lt $t.Item3) -and ($t.Item2 -lt $t.Item3))
			{
				$t = [ValueTuple[int,int,int]]::new(
					$t.Item3,
					$t.Item1,
					$t.Item2
				)
			}
			$tris[$i] = $t
		}
	}

	Orient-Triangles $meshA.tris
	Orient-Triangles $meshB.tris

	function Find-Triangle {
		param(
			[Collections.Generic.List[ValueTuple[int, int, int]]]$tris,
			[ValueTuple[int, int, int]]$tri
		)
		for ($i = 0; $i -lt $tris.Count; $i++) {
			$t = $tris[$i]
			if (($t.Item1 -eq $tri.Item1) -and ($t.Item2 -eq $tri.Item2) -and ($t.Item3 -eq $tri.Item3)) {
				return $i
			}
		}
		return -1
	}

	$triPairFound = 0;
	for ($i = $meshA.tris.Count - 1; $i -ge 0; $i--)
	{
		$idx = Find-Triangle -Tris $meshB.tris -Tri $meshA.tris[$i]
		if ($idx -ge 0) {
			$meshA.tris.RemoveAt($i)
			$meshB.tris.RemoveAt($idx)
			$triPairFound++;
		}
	}

	Write-Verbose "$triPairFound matching triangles found; $($meshA.tris.Count) & $($meshB.tris.Count) triangles yet unmatched"

}

function CompareAndRemoveQuads
{
	param(
		[pscustomobject]$meshA,
		[pscustomobject]$meshB
	)
	# Removes all triangle-pairs froming quads from A and B which are present in both

	Write-Verbose "Fancy quad matching required"

}

try {

	$meshA = Load-WavefrontObjFile $FileA
	$meshB = Load-WavefrontObjFile $FileB

	CompareVertices $meshA $meshB
	Write-Verbose "Vertices are equal"

	CompareAndRemoveTriangles $meshA $meshB

	if (($meshA.tris.Count -gt 0) -and ($meshB.tris.Count -gt 0)) {
		CompareAndRemoveQuads $meshA $meshB
	}

	if (($meshA.tris.Count -gt 0) -or ($meshB.tris.Count -gt 0)) {
		throw "$($meshA.tris.Count) unmatched triangles in Mesh A and $($meshB.tris.Count) unmatched triangles in Mesh B; should be both zero if equal"
	}

	Write-Host "equal" -ForegroundColor green -BackgroundColor black
	exit 0
}
catch {
	Write-Error $_ -ErrorAction Continue
	Write-Host "notequal" -ForegroundColor red -BackgroundColor black
	exit 1
}
