--
-- Test script
-- when run, generates "test-obj.obj" with two cubes at defined positions
--
-- Run with:
--   MeshProc.exe run $(SolutionDir)test\gen-test-obj.lua -v
--
meshprocVersion.assertOrNewer(0, 5, 0)
meshprocVersion.assertOlderThan(0, 6, 0)

local xyz_math = require("xyz_math")

local make = meshproc.generator.Cuboid.new()

make:set("SizeX", 2)
make:set("SizeY", 4)
make:set("SizeZ", 6)
make:invoke()
local cube1 = make:get("Mesh")

make:set("SizeX", 2)
make:set("SizeY", 1)
make:set("SizeZ", 0.5)
make:invoke()
local cube2 = make:get("Mesh")

local scene = meshproc.Scene.new()
scene:place(cube1, XMat4.translate(-1, -2, -3))
scene:place(cube2, XMat4.translate(3, -0.5, -0.25))

local file = meshproc.io.ObjWriter.new()
file:set("Scene", scene)
file:set("Path", "test-obj.obj")
file:invoke()
