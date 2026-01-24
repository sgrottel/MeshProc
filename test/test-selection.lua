
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
	local make = meshproc.generator.SphereIco.new()
	log.detail("Creating "..tostring(make))
	make["Iterations"] = 2
	make:invoke()
	mesh = make["Mesh"]

	local p = meshproc.HalfSpace.new()
	local a, b = p:get()
	log.detail("HalfSpace: ("..tostring(a.x)..", "..tostring(a.y)..", "..tostring(a.z).."), "..tostring(b))

	p:set(XVec3(1, 2, 0), 3)
	a, b = p:get()
	log.detail("HalfSpace: ("..tostring(a.x)..", "..tostring(a.y)..", "..tostring(a.z).."), "..tostring(b))

	p:set(XVec3(0, 2, 0), XVec3(1, 1, 1))
	a, b = p:get()
	log.detail("HalfSpace: ("..tostring(a.x)..", "..tostring(a.y)..", "..tostring(a.z).."), "..tostring(b))

	a = XVec3(2, 2, 2)
	log.detail("HalfSpace dist to ("..tostring(a.x)..", "..tostring(a.y)..", "..tostring(a.z)..") = "..tostring(p:dist(a)))

	a = XVec3(-2, -3, 2)
	log.detail("HalfSpace dist to ("..tostring(a.x)..", "..tostring(a.y)..", "..tostring(a.z)..") = "..tostring(p:dist(a)))

	p:set(XVec3(0, 0, 1), -0.1)

	local mesh2 = mesh:clone()

	local cut = meshproc.edit.CutHalfSpace.new()
	cut["Mesh"] = mesh
	cut["HalfSpace"] = p
	cut:invoke()

	local scene = meshproc.Scene.new()

	scene:place(mesh)
	scene:place(mesh2, XMat4.translate(0, 3, 0))

	mesh = scene:bake()
end

-- explicitly collect the no longer used "make" object
collectgarbage("collect")

local meshval = nil
do
	local sel = meshproc.IndexList.new()
	for i = 1, 4 do
		local v = XVec3(0, 0, 0)
		local x
		while (math.abs((v.x * v.x + v.y * v.y + v.z * v.z) - 1) > 0.0001) or (v.z < 0.1) do
			x = math.random(#mesh.vertex)
			v = mesh.vertex[x]
		end
		sel:insert(x)
		-- mesh.vertex[x] = mesh.vertex[x] * 4
	end

	local dists = meshproc.compute.VertexEdgeDistance.new()
	dists.Mesh = mesh
	dists.Selection = sel
	dists:invoke()
	meshval = dists.Distances
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

	for vI, v in ipairs(mesh.vertex) do
		mesh.vertex[vI] = v + normals[vI] * meshval[vI] * 0.2
	end
end

-- save scene to file
do
	local scenecol = meshproc.Vec3ListList.new()
	scenecol:insert(meshcol)

	local file = meshproc.io.ObjWriter.new()
	file.Scene = mesh -- alternative syntax to the array operators
	file.VertexColors = scenecol
	file.Path = "test-selection.obj"
	file:invoke()
end
