--
-- This script is a wild playground during development
-- It's not ment as an example or as a test or anything
--
-- run demo.lua -v -arg sizex=3 -arg hello
--

-- compatible with MeshProc v0.5.x
meshprocVersion.assertOrNewer(0, 5, 0)
meshprocVersion.assertOlderThan(0, 6, 0)

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
	make:set("SizeX", tonumber(args.sizex) or error("arg.sizex is not a number"))
	make:invoke()
	cube = make:get("Mesh")
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

-- scene:place(poly, XMat4.translate(0, 0, 2))

-- save scene to file
do
	local stl = meshproc.io.StlWriter.new()
	stl:set("Scene", scene)
	stl:set("Path", "out.stl")
	stl:invoke()
end
