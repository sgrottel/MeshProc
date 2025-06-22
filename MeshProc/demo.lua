--
-- This script is a wild playground during development
-- It's not ment as an example or as a test or anything
--
-- run demo.lua -v -arg sizex=3 -arg hello
--

local xyz_math = require("xyz_math")

log.write("Test script")

local argsStr = 'args:'
for k, v in pairs(args) do
	argsStr = argsStr .. '\n  "' .. k .. '" = "' .. v .. '"';
end
log.detail(argsStr)
if args.sizex then
	log.detail("args.sizex is set")
end
if not args.nope then
	log.detail("args.not is not set")
end

local dev = meshproc._createCommand("DevPlayground");
dev:invoke();

log.warn("破滅");

cube = meshproc.generator.Cube.new()

log.write("cube.x = " .. tostring(cube:get("SizeX")))
log.write("cube.y = " .. tostring(cube:get("SizeY")))
log.write("cube.z = " .. tostring(cube:get("SizeZ")))
log.write("mesh = " .. tostring(cube:get("Mesh")))

-- Accessing a field that does not exist with stop the script with a critical error
-- log.write("cube.invalid = " .. tostring(cube:get("invalid")))

cube:set("SizeX", tonumber(args.sizex or 4))
log.write("cube.x = " .. tostring(cube:get("SizeX")))

if cube:invoke() then
	log.write("Cube generated")
else
	log.error("Cube failed")
end

log.write("mesh = " .. tostring(cube:get("Mesh")))

local mesh = cube:get("Mesh");

local scene = meshproc.Scene.new()
scene:place(mesh);

local mat = XMat4.translate(0, 0, 2) * XMat4.rotation_z(math.pi/4) * XMat4.scale(1, 2, 1)
scene:place(mesh, mat);

scene:place(mesh, mat * XMat4.translate(5, 0, 0));

ply = meshproc.io.PlyWriter.new()
ply:set("Path", "out.ply")
ply:set("Scene", scene)
log.write("ply.path = " .. tostring(ply:get("Path")))
ply:invoke()

local close = meshproc.CloseLoopWithPin.new()

local vec = XVec3(2, 2, 4)
log.write("vec = " .. tostring(vec.x or 0) .. ", " .. tostring(vec.y or 0) .. ", " .. tostring(vec.z or 0))

vec = close:get("PinOffset")
log.write("vec = " .. tostring(vec.x or 0) .. ", " .. tostring(vec.y or 0) .. ", " .. tostring(vec.z or 0))

close:set("PinOffset", XVec3(1, 2, 3))
vec = close:get("PinOffset")
log.write("vec = " .. tostring(vec.x or 0) .. ", " .. tostring(vec.y or 0) .. ", " .. tostring(vec.z or 0))

close:set("PinOffset", XVec2(4, 5))
vec = close:get("PinOffset")
log.write("vec = " .. tostring(vec.x or 0) .. ", " .. tostring(vec.y or 0) .. ", " .. tostring(vec.z or 0))

close:set("PinOffset", XVec4(7, 8, 9, 0))
vec = close:get("PinOffset")
log.write("vec = " .. tostring(vec.x or 0) .. ", " .. tostring(vec.y or 0) .. ", " .. tostring(vec.z or 0))

close:set("PinOffset", XVec4(10, 11, 12, 1))
vec = close:get("PinOffset")
log.write("vec = " .. tostring(vec.x or 0) .. ", " .. tostring(vec.y or 0) .. ", " .. tostring(vec.z or 0))

close:set("PinOffset", XVec4(8, 4, 6, 2))
vec = close:get("PinOffset")
log.write("vec = " .. tostring(vec.x or 0) .. ", " .. tostring(vec.y or 0) .. ", " .. tostring(vec.z or 0))
