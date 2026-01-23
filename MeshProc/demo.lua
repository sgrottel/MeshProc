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

end

-- explicitly collect the no longer used "make" object
collectgarbage("collect")

-- do return end


-- -- 2D Shapes
-- local shape = meshproc.Shape2D.new()
-- shape:add(XVec2(-1,0))
-- shape:add(XVec2(1,0))
-- shape:add(XVec2(0,2))
-- 
-- shape:add(XVec2(2, 0), 2)
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
local scene = meshproc.Scene.new()

scene:place(mesh)
-- scene:place(mesh2, XMat4.translate(0, 3, 0))

-- save scene to file
do
	local stl = meshproc.io.StlWriter.new()
	stl.Scene = scene -- alternative syntax to the array operators
	stl.Path = "out.stl"
	stl:invoke()
end
