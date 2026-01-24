--
-- This script is a wild playground during development
-- It's not ment as an example or as a test or anything
--
-- run demo.lua -v
--

-- compatible with MeshProc v0.5.x
meshproc.Version.assert_or_newer(0, 5, 0)
meshproc.Version.assert_older_than(0, 6, 0)
log.detail("MeshProc v"..tostring(meshproc.Version.get()).." ["..tostring(meshproc.Version.get()[1]).."."..tostring(meshproc.Version.get()[2]).."]")
math.randomseed(1236)

-- load math library
local xyz_math = require("xyz_math")
log.write("Test script")

local mesh = nil
do
	local reader = meshproc.io.PlyReader.new()
	reader.Path = "../test/test-open-torus.ply"
	reader:invoke()
	mesh = reader.Mesh
end

do
	local scene = meshproc.Scene.new()
	scene:place(mesh)
	scene:place(mesh, XMat4.translate(3, 0, -3) * XMat4.rotation_y(math.rad(180)) * XMat4.rotation_z(math.rad(90)) * XMat4.translate(-3, 0, 0))
	scene:place(mesh, XMat4.translate(-10, 0, 0))
	mesh = scene:bake()
end

local mesh2 = nil
do
	local p = meshproc.compute.ProjectionScarf.new()
	p.Mesh = mesh
	local pp = meshproc.HalfSpace.new()
	pp:set(XVec3(1, 0.2, 0), XVec3(0, 0, 0))
	p.Projection = pp
	p:invoke()
	mesh2 = p.OutMesh
end

-- save scene to file
do
	local file = meshproc.io.ObjWriter.new()

	file.Scene = mesh
	file.Path = "out.obj"
	file:invoke()

	file.Scene = mesh2
	file.Path = "out2.obj"
	file:invoke()
end
