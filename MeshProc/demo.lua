--
-- This script is a wild playground during development
-- It's not ment as an example or as a test or anything
--
-- run demo.lua -v -arg sizex=3 -arg hello
--

-- compatible with MeshProc v0.5.x
meshproc.Version.assert_or_newer(0, 5, 0)
meshproc.Version.assert_older_than(0, 6, 0)
log.detail("MeshProc v"..tostring(meshproc.Version.get()).." ["..tostring(meshproc.Version.get()[1]).."."..tostring(meshproc.Version.get()[2]).."]")

-- load math library
local xyz_math = require("xyz_math")
log.write("Test script")

log.write("Call arg sizex: " .. tostring(args.sizex) .. " [" .. type(args.sizex) .. "]") -- is a string "3" from the default command line above
log.write("Call arg hello: " .. tostring(args.hello) .. " [" .. type(args.hello) .. "]") -- is an empty string from the default command line above
log.write("Call arg never: " .. tostring(args.never) .. " [" .. type(args.never) .. "]") -- is "nil" if not specified in the command line

-- create a cuboid mesh [0..sizex][0..1][0..1]
local cube = nil
do
	local make = meshproc.generator.Cuboid.new()
	log.detail("Creating "..tostring(make))
	make["SizeX"] = (tonumber(args.sizex) or error("arg.sizex is not a number"))
	make:invoke()
	cube = make["Mesh"]

	log.detail(tostring(cube))
	log.detail(tostring(cube.vertex))
end

-- explicitly collect the no longer used "make" object
collectgarbage("collect")

-- create an icosahedron mesh
local ico = nil
local croncle = nil
do
	local make = meshproc.generator.Icosahedron.new()
	make:invoke()
	ico = make["Mesh"]

	make:invoke()
	croncle = make["Mesh"]

	do
		local vertNorms = meshproc.compute.VertexNormals.new()
		local move = meshproc.edit.DisplacementNoise.new()
		local subdiv = meshproc.edit.Subdivision.new()

		vertNorms["Mesh"] = croncle
		move["Mesh"] = croncle
		subdiv["Mesh"] = croncle
		move["Seed"] = 0815
		move["Min"] = 0
		move["Max"] = 0.1

		vertNorms:invoke()
		move["Dirs"] = vertNorms["Normals"]
		move:invoke()
		subdiv:invoke()

		move["Min"] = -0.1
		move["Max"] = 0.3
		vertNorms:invoke()
		move["Dirs"] = vertNorms["Normals"]
		move:invoke()
		subdiv:invoke()

		move["Min"] = -0.2
		move["Max"] = 0.2
		vertNorms:invoke()
		move["Dirs"] = vertNorms["Normals"]
		move:invoke()

		log.detail(tostring(#(vertNorms["Normals"]) == croncle:vertex_length()))

	end

	ico:apply_transform(XMat4.translate(0.5, 1, 3))

	local minBB, maxBB = ico:calc_boundingbox()
	log.write("Ico bbox: { "..tostring(minBB.x)..", "..tostring(minBB.y)..", "..tostring(minBB.z).." } - { "..tostring(maxBB.x)..", "..tostring(maxBB.y)..", "..tostring(maxBB.z).." }")

	minBB.x = minBB.x + 0.01

	local vsel = meshproc.IndexList.new()
	vsel:insert(2)  -- list is [2]
	vsel:insert(3)  -- insert at the end; list is [2, 3]
	vsel:insert(2, 4)  -- inserts entry "4" right before the second entry in the list; list is [2, 4, 3]
	vsel:insert(8)  -- insert at the end; list is [2, 4, 3, 8]
	vsel:remove(2)  -- remove at 2; list is [2, 3, 8]
	vsel:remove()  -- remove last; list is [2, 3]
	vsel:resize(4)  -- resized and list is now [2, 3, 0, 0] with invalid entries "0"
	vsel[4] = 42  -- set access; list is now [2, 3, 0, 42]
	for i, v in ipairs(vsel) do log.detail("vsel["..i.."] = "..v) end -- iterating list
	vsel:remove(3) -- list is now [2, 3, 42]
	log.write(tostring(#vsel)) -- size of list
	for i, v in ipairs(vsel) do log.detail("vsel["..i.."] = "..v) end -- iterating list

	vsel:resize(0)  -- clear

	for i = 1, ico:vertex_length() do
		local v = ico:vertex_get(i)
		if (v.x < minBB.x) then
			vsel:insert(i)
		end
	end
	for i, v in ipairs(vsel) do log.detail("vsel["..i.."] = "..v) end

	-- ico:vertex_remove(vsel) -- remove multiple vertices (and connected triangles)
	-- ico:vertex_remove(2) -- remove vertex 3 (and connected triangles)
	-- ico:vertex_remove() -- remove last vertex in list (and connected triangles)

end

-- explicitly collect the no longer used "make" object
collectgarbage("collect")

-- do return end

-- create an octahedron-based star mesh
local oct = nil
do
	local make = meshproc.generator.Octahedron.new()
	make:invoke()
	oct = make["Mesh"]

	-- in-place edit `oct`
	local subdiv = meshproc.edit.Subdivision.new()
	subdiv["Mesh"] = oct
	subdiv:invoke()

	local vCnt = oct:vertex_length()
	local tCnt = oct:triangle_length()
	log.write("Oct mesh: " .. tostring(vCnt) .. "v " .. tostring(tCnt) .. "t")

	for vI = 1, vCnt do
		local v = oct:vertex_get(vI)
		log.detail("v["..tostring(vI).."] = { "..tostring(v.x)..", "..tostring(v.y)..", "..tostring(v.z).." }")

		if (v.x == 0 and v.y == 0) or (v.x == 0 and v.z == 0) or (v.y == 0 and v.z == 0) then
			oct:vertex_set(vI, v * 3)
		end
	end

	for tI = 1, tCnt do
		local t = oct:triangle_get(tI)
		log.detail("t["..tostring(tI).."] = { "..tostring(t.x)..", "..tostring(t.y)..", "..tostring(t.z).." }")
		-- for debugging, the next lines change winding of triangle
		--t.y, t.z = t.z, t.y
		--oct:triangle_set(tI, t)
	end

end

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

scene:place(cube)
scene:place(cube, XMat4.translate(0, 2, 0)) -- second instance of 'cube' translated to +y

scene:place(ico)

scene:place(oct, XMat4.translate(2.5, 4, 3))

scene:place(croncle, XMat4.translate(-2, 2, 1))

-- scene:place(poly, XMat4.translate(0, 0, 2))

-- save scene to file
do
	local stl = meshproc.io.StlWriter.new()
	stl.Scene = scene -- alternative syntax to the array operators
	stl.Path = "out.stl"
	stl:invoke()
end
