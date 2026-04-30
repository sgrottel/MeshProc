-- compatible with MeshProc v0.6.x
meshproc.Version.assert_or_newer(0, 6, 0)
meshproc.Version.assert_older_than(0, 7, 0)
log.detail("MeshProc v"..tostring(meshproc.Version.get()).." ["..tostring(meshproc.Version.get()[1]).."."..tostring(meshproc.Version.get()[2]).."]")
local xyz_math = require("xyz_math")

local mesh = nil
do -- load
	local reader = meshproc.io.StlReader.new()
	reader.Path = "test-torus.stl"
	reader:invoke()
	mesh = reader.Mesh
end


do -- cut
	local cut = meshproc.edit.CutPlaneLoop.new()
	cut.Mesh = mesh
	cut.Plane = meshproc.HalfSpace.new()

	cut.Point = XVec3(-2.8, 2.8, 0) -- point
	cut.Plane:set( XVec3(-1, -1, 0.01), cut.Point) -- normal

	cut:invoke()
end


local meshval = nil
do
	local vedtc = meshproc.compute.VertexEdgeDistanceToCut.new()
	vedtc.Mesh = mesh
	vedtc.PlaneOrigin = XVec3(-3.5, 0, 0)
	vedtc.PlaneNormal = XVec3(-1, -0.1, 0)
	vedtc.PlaneXAxis = XVec3(0, 1, 0)
	vedtc.PlaneRectWidth = 10
	vedtc.PlaneRectHeight = 10
	vedtc:invoke()
	meshval = vedtc.Distances
end

local meshcol = nil
do
	local colmap = meshproc.compute.LinearColorMap.new()
	colmap.Scalars = meshval
	colmap:invoke()
	meshcol = colmap.Colors
end

-- save scene to file
do
	local scenecol = meshproc.Vec3ListList.new()
	scenecol:insert(meshcol)

	local file = meshproc.io.ObjWriter.new()
	file.Scene = mesh
	file.VertexColors = scenecol
	file.Path = "test-cutplaneloop.obj"
	file:invoke()
end
