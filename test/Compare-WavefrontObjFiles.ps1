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

	# Assumption: Quads are defined by exactly two triangles, that share exactly two vertices, and have the exact same face normal vector

	# Assumption: all remaining triangles are part of such quads

	function Build-QuadList {
		param(
			[pscustomobject]$mesh
		)

		# normales are fix-point quantized to 100th, i.e.
		#     -1 .. 0 .. 1
		#  =>  0 .. 100 .. 200
		# x y z => (x * 200 + y) * 200 + z
		$normalKey = [Collections.Generic.List[int]]::new()
		for ($i = 0; $i -lt $mesh.tris.Count; $i++)
		{
			$v1 = $mesh.verts[$mesh.tris[$i].Item1]
			$v2 = $mesh.verts[$mesh.tris[$i].Item2]
			$v3 = $mesh.verts[$mesh.tris[$i].Item3]

			# Edge vectors
			$e1 = [ValueTuple[float,float,float]]::new(
				$v2.Item1 - $v1.Item1,
				$v2.Item2 - $v1.Item2,
				$v2.Item3 - $v1.Item3
			)

			$e2 = [ValueTuple[float,float,float]]::new(
				$v3.Item1 - $v1.Item1,
				$v3.Item2 - $v1.Item2,
				$v3.Item3 - $v1.Item3
			)

			# Cross product
			$nx = $e1.Item2 * $e2.Item3 - $e1.Item3 * $e2.Item2
			$ny = $e1.Item3 * $e2.Item1 - $e1.Item1 * $e2.Item3
			$nz = $e1.Item1 * $e2.Item2 - $e1.Item2 * $e2.Item1

			# Normalize
			$len = [math]::Sqrt($nx*$nx + $ny*$ny + $nz*$nz)

			if ($len -gt 0) {
				$normal = [ValueTuple[float,float,float]]::new(
					[float]([math]::Round($nx / $len, 2)),
					[float]([math]::Round($ny / $len, 2)),
					[float]([math]::Round($nz / $len, 2))
				)
			}
			else {
				# Degenerate triangle: zero normal
				$normal = [ValueTuple[float,float,float]]::new(0.0, 0.0, 0.0)
			}

			$normalKeyX = [math]::Clamp(100 + $normal.Item1 * 100, 0, 200)
			$normalKeyY = [math]::Clamp(100 + $normal.Item2 * 100, 0, 200)
			$normalKeyZ = [math]::Clamp(100 + $normal.Item3 * 100, 0, 200)

			[void]$normalKey.Add(($normalKeyX * 200 + $normalKeyY) * 200 + $normalKeyZ)
		}

		function Get-MatchingComponentCount {
			param(
				[ValueTuple[int,int,int]]$a,
				[ValueTuple[int,int,int]]$b
			)

			$count = 0
			if ($a.Item1 -eq $b.Item1 -or $a.Item1 -eq $b.Item2 -or $a.Item1 -eq $b.Item3) { $count++ }
			if ($a.Item2 -eq $b.Item1 -or $a.Item2 -eq $b.Item2 -or $a.Item2 -eq $b.Item3) { $count++ }
			if ($a.Item3 -eq $b.Item1 -or $a.Item3 -eq $b.Item2 -or $a.Item3 -eq $b.Item3) { $count++ }

			return $count
		}

		$pairs = [System.Collections.Generic.List[ValueTuple[int,int]]]::new()

		for ($i = 0; $i -lt $mesh.tris.Count; $i++) {
			for ($j = $i + 1; $j -lt $mesh.tris.Count; $j++) {

				# Must have same normal key
				if ($normalKey[$i] -ne $normalKey[$j]) { continue }

				# Must share exactly two vertex indices
				if ((Get-MatchingComponentCount $mesh.tris[$i] $mesh.tris[$j]) -eq 2) {
					[void]$pairs.Add([ValueTuple[int,int]]::new($i, $j))
				}
			}
		}

		$used = [System.Collections.Generic.HashSet[int]]::new()
		$filtered = [System.Collections.Generic.List[ValueTuple[int,int]]]::new()

		foreach ($p in $pairs) {
			$a = $p.Item1
			$b = $p.Item2

			if (-not $used.Contains($a) -and -not $used.Contains($b)) {
				[void]$filtered.Add($p)
				[void]$used.Add($a)
				[void]$used.Add($b)
			}
		}

		$quads = @{}

		# note: we build not geometrically valid quads, only collect the four indices in arbitrary order
		foreach ($p in $filtered) {
			$t1 = $mesh.tris[$p.Item1]
			$t2 = $mesh.tris[$p.Item2]

			$q = @(
				$t1.Item1, $t1.Item2, $t1.Item3,
				$t2.Item1, $t2.Item2, $t2.Item3
			) | Sort-Object -Unique
			if ($q.Count -ne 4) { throw "Quad collection returned wrong number of vertex indices $quad" }

			$quad = [ValueTuple[int,int,int,int]]::new($q[0], $q[1], $q[2], $q[3])
			$quads[$quad] = $p
		}

		return $quads
	}

	$quadsA = Build-QuadList $meshA
	$meshB.verts = $meshA.verts
	$quadsB = Build-QuadList $meshB
	$meshB.verts = $null

	$delInA = @()
	$delInB = @()

	foreach ($quadKey in $quadsA.Keys) {
		if ($quadsB.Contains($quadKey)) {
			$trisA = $quadsA[$quadKey]
			$trisB = $quadsB[$quadKey]

			$delInA += $trisA.Item1
			$delInA += $trisA.Item2

			$delInB += $trisB.Item1
			$delInB += $trisB.Item2
		}
	}

	$delInA | Sort-Object -Unique -Descending | % { $meshA.tris.RemoveAt($_) }
	$delInB | Sort-Object -Unique -Descending | % { $meshB.tris.RemoveAt($_) }
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
