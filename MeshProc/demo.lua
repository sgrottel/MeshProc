--
-- This script is a wild playground during development
-- It's not ment as an example or as a test or anything
--
-- run demo.lua -v -arg sizex=3 -arg hello
--
local xyz_math = require("xyz_math")
log.write("Test script")

local make = meshproc.generator.Cube.new()
make:invoke()
local cube = make:get("Mesh") -- cube [0..1]

-- 2D Shapes
local shape = meshproc.Shape2D.new()
shape:add(XVec2(-1,0))
shape:add(XVec2(1,0))
shape:add(XVec2(0,2))

shape:add(XVec2(2, 0), 2)
shape:add(XVec2(2, 2), 2)
shape:add(XVec2(4, 2), 2)
shape:add(XVec2(4, 0), 2)

shape:add(XVec2(3, 0.5), 3)
shape:add(XVec2(2.5, 1), 3)
-- strange order of verties on purpose to have an intersection in the loops
shape:add(XVec2(3.5, 1), 3)
shape:add(XVec2(3, 1.5), 3)

csv = meshproc.io.CsvShape2DWriter.new()
csv:set("Shape", shape)
csv:set("Path", "shape2d.csv")
csv:invoke()

make = meshproc.generator.LinearExtrude2DMesh.new()
make:set("Shape2D", shape)
make:invoke()
local poly = make:get("Mesh")

-- TODO: Subtract

local scene = meshproc.Scene.new()
scene:place(cube)
scene:place(poly, XMat4.translate(0, 0, 2))

local stl = meshproc.io.StlWriter.new()
stl:set("Scene", scene)
stl:set("Path", "out.stl")
stl:invoke()
