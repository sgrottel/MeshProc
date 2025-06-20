log.write("Test script")

log.warn("破滅");

cube = meshproc.generator.Cube.new()

log.write("cube.x = " .. cube:get("SizeX"))
log.write("cube.y = " .. cube:get("SizeY"))
log.write("cube.z = " .. cube:get("SizeZ"))
--log.write("cube.mesh = " .. cube:get("Mesh"))

--log.write("cube.invalid = " .. cube:get("invalid"))

cube:set("SizeX", 4)
log.write("cube.x = " .. cube:get("SizeX"))

if cube:invoke() then
	log.write("Cube generated")
else
	log.error("Cube failed")
end

ply = meshproc.io.PlyWriter.new()
ply:set("Path", "out.ply")
log.write("ply.path = " .. ply:get("Path"))

--log.write("cube.mesh = " .. cube:get("Mesh"))
