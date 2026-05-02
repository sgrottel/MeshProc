-- compatible with MeshProc v0.6.x
meshproc.Version.assert_or_newer(0, 6, 0)
meshproc.Version.assert_older_than(0, 7, 0)
log.detail("MeshProc v"..tostring(meshproc.Version.get()).." ["..tostring(meshproc.Version.get()[1]).."."..tostring(meshproc.Version.get()[2]).."]")
local xyz_math = require("xyz_math")

local mesh1 = nil
local mesh2 = nil
do -- load
	local reader = meshproc.io.StlReader.new()
	reader.Path = "test-torus.stl"
	reader:invoke()
	mesh1 = reader.Mesh
	mesh2 = mesh1:clone()
end


do -- cut
	local cut = meshproc.edit.CutPlaneLoop.new()
	cut.Mesh = mesh1
	cut.Plane = meshproc.HalfSpace.new()

	cut.Point = XVec3(-2.8, 2.8, 0) -- point
	cut.Plane:set( XVec3(-1, -1, 0.01), cut.Point) -- normal

	cut:invoke()

	cut.Mesh = mesh2
	cut.Plane:set( XVec3(-1, -1, 0), cut.Point) -- normal
	cut:invoke()
end


local mesh1col = nil
local mesh2col = nil
do
	local vedtc = meshproc.compute.VertexEdgeDistanceToCut.new()
	local colmap = meshproc.compute.LinearColorMap.new()

	vedtc.Mesh = mesh1
	vedtc.PlaneOrigin = XVec3(-3.5, 0, 0)
	vedtc.PlaneNormal = XVec3(-1, -0.1, 0)
	vedtc.PlaneXAxis = XVec3(0, 1, 0)
	vedtc.PlaneRectWidth = 10
	vedtc.PlaneRectHeight = 10
	vedtc:invoke()
	colmap.Scalars = vedtc.Distances
	colmap:invoke()
	mesh1col = colmap.Colors

	vedtc.Mesh = mesh2
	vedtc:invoke()
	colmap.Scalars = vedtc.Distances
	colmap:invoke()
	mesh2col = colmap.Colors

end

-- save scene to file
do
	local scene = meshproc.Scene.new()
	scene:place(mesh1)
	scene:place(mesh2, XMat4.translate(10, 0, 0))

	local scenecol = meshproc.Vec3ListList.new()
	scenecol:insert(mesh1col)
	scenecol:insert(mesh2col)

	local file = meshproc.io.ObjWriter.new()
	file.Scene = scene
	file.VertexColors = scenecol
	file.Path = "test-cutplaneloop.obj"
	file:invoke()
end
