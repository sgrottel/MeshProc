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

local meshval = nil
do
	local vedtc = meshproc.compute.VertexEdgeDistanceToCut.new()

	vedtc.Mesh = mesh
	vedtc.PlaneOrigin = XVec3(0, 0, 0)
	vedtc.PlaneNormal = XVec3(0, 0, -1)
	vedtc.PlaneXAxis = XVec3(1, 0, 0)
	vedtc.PlaneRectWidth = 2.2
	vedtc.PlaneRectHeight = 2.2

	vedtc:invoke()

	meshval = vedtc.Distances
end

-- explicitly collect the no longer used "make" object
-- collectgarbage("collect")

local meshcol = nil
do
	local colmap = meshproc.compute.LinearColorMap.new()
	colmap.Scalars = meshval
	colmap:invoke()
	meshcol = colmap.Colors
end

do
	log.write("Deforming mesh")
	local normals = meshproc.compute.VertexNormals.new()
	normals.Mesh = mesh
	normals:invoke()
	normals = normals.Normals

	local minv = meshval[1]
	local maxv = meshval[1]
	for i = 2, #meshval do
		local v = meshval[i]
		if v < minv then
			minv = v
		elseif v > maxv then
			maxv = v
		end
	end
	log.detail("Dist values in "..tostring(minv).." .. "..tostring(maxv))

	for vI, v in ipairs(mesh.vertex) do
		local vv = meshval[vI]
		if vv > 0 then
			vv = vv / maxv
		else
			vv = -(vv / minv)
		end
		mesh.vertex[vI] = v + normals[vI] * vv * 0.6
	end
end

-- -- do return end
-- 
-- 
-- -- -- 2D Shapes
-- -- local shape = meshproc.Shape2D.new()
-- -- shape:add(XVec2(-1,0))
-- -- shape:add(XVec2(1,0))
-- -- shape:add(XVec2(0,2))
-- -- 
-- -- shape:add(XVec2(2, 0), 2)
-- shape:add(XVec2(2, 2), 2)
-- shape:add(XVec2(4, 2), 2)
-- shape:add(XVec2(4, 0), 2)
-- 
-- shape:add(XVec2(3, 0.5), 3)
-- shape:add(XVec2(2.5, 1), 3)
-- -- strange order of verties on purpose to have an intersection in the loops
-- shape:add(XVec2(3.5, 1), 3)
-- shape:add(XVec2(3, 1.5), 3)
-- 
-- csv = meshproc.io.CsvShape2DWriter.new()
-- csv:set("Shape", shape)
-- csv:set("Path", "shape2d.csv")
-- csv:invoke()
-- 
-- make = meshproc.generator.LinearExtrude2DMesh.new()
-- make:set("Shape2D", shape)
-- make:invoke()
-- local poly = make:get("Mesh")
--
-- -- TODO: Subtract


-- compose scene from meshes
-- local scene = meshproc.Scene.new()
-- scene:place(mesh)
-- scene:place(mesh2, XMat4.translate(0, 3, 0))

-- save scene to file
do
	local scenecol = meshproc.Vec3ListList.new()
	scenecol:insert(meshcol)

	local file = meshproc.io.ObjWriter.new()
	file.Scene = mesh
	file.VertexColors = scenecol
	file.Path = "out.obj"
	file:invoke()
end
