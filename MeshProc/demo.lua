log.write("Test script")

log.warn("破滅");

cube = meshproc.generator.Cube.new()

log.write("cube.x = " .. tostring(cube:get("SizeX")))
log.write("cube.y = " .. tostring(cube:get("SizeY")))
log.write("cube.z = " .. tostring(cube:get("SizeZ")))
log.write("mesh = " .. tostring(cube:get("Mesh")))

-- Accessing a field that does not exist with stop the script with a critical error
-- log.write("cube.invalid = " .. tostring(cube:get("invalid")))

cube:set("SizeX", 4)
log.write("cube.x = " .. tostring(cube:get("SizeX")))

if cube:invoke() then
	log.write("Cube generated")
else
	log.error("Cube failed")
end

log.write("mesh = " .. tostring(cube:get("Mesh")))

place = meshproc.PlaceMesh.new()
place:set("Mesh", cube:get("Mesh"))
place:invoke()

ply = meshproc.io.PlyWriter.new()
ply:set("Path", "out.ply")
ply:set("Scene", place:get("Scene"))
log.write("ply.path = " .. tostring(ply:get("Path")))
ply:invoke()

