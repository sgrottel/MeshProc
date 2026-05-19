-- compatible with MeshProc v0.6.x
meshproc.Version.assert_or_newer(0, 6, 0)
meshproc.Version.assert_older_than(0, 7, 0)
local xyz_math = require("xyz_math")
math.randomseed(12345)

local mesh = nil
do -- load
	local reader = meshproc.io.StlReader.new()
	reader.Path = "test-outside-surface.stl"
	reader:invoke()
	mesh = reader.Mesh
end

local meshval = nil
do
	local classifier = meshproc.compute.OutsideSurfaceClassification.new()
	classifier.Mesh = mesh
	classifier:invoke()
	local recolor = meshproc.compute.VertexColorFromTriangleColor.new()
	recolor.Mesh = mesh
	recolor.TriangleColors = classifier.FaceType
	recolor:invoke()
	meshval = recolor.VertexColors
end

local meshcol = nil
do
	local colmap = meshproc.compute.LinearColorMap.new()
	colmap.Scalars = meshval
	colmap:invoke()
	meshcol = colmap.Colors
end

do
	local scenecol = meshproc.Vec3ListList.new()
	scenecol:insert(meshcol)

	local file = meshproc.io.ObjWriter.new()
	file.Scene = mesh
	file.VertexColors = scenecol
	file.Path = "test-outside-surface.obj"
	file:invoke()
end
