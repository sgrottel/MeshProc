--
-- Test script
-- when run, generates "test-obj.obj" with two cubes at defined positions
--
-- Run with:
--   MeshProc.exe run $(SolutionDir)test\gen-test-obj.lua -v
--
meshproc.Version.assert_or_newer(0, 5, 0)
meshproc.Version.assert_older_than(0, 6, 0)

local xyz_math = require("xyz_math")

local make = meshproc.generator.Cuboid.new()

make["SizeX"] = 2
make["SizeY"] = 4
make["SizeZ"] = 6
make:invoke()
local cube1 = make["Mesh"]

make["SizeX"] = 2
make["SizeY"] = 1
make["SizeZ"] = 0.5
make:invoke()
local cube2 = make["Mesh"]

local scene = meshproc.Scene.new()
scene:place(cube1, XMat4.translate(-1, -2, -3))
scene:place(cube2, XMat4.translate(3, -0.5, -0.25))

local file = meshproc.io.ObjWriter.new()
file["Scene"] = scene
file["Path"] = "test-obj.obj"
file:invoke()
