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
	reader.Path = "test-open-torus.ply"
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

-- save scene to file
do
	local scenecol = meshproc.Vec3ListList.new()
	scenecol:insert(meshcol)

	local file = meshproc.io.ObjWriter.new()
	file.Scene = mesh
	file.VertexColors = scenecol
	file.Path = "test-VertexEdgeDistanceToCut.obj"
	file:invoke()
end
