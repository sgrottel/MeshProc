log.write("Test script")

cube = meshproc._createCommand("generator.Cube");

log.write("cube.x = " .. cube:get("SizeX"))
log.write("cube.y = " .. cube:get("SizeY"))
log.write("cube.z = " .. cube:get("SizeZ"))
log.write("cube.mesh = " .. cube:get("Mesh"))

log.write("cube.invalid = " .. cube:get("invalid"))

cube:set("SizeX", 4);
log.write("cube.x = " .. cube:get("SizeX"))

if cube:invoke() then
	log.write("Cube generated")
else
	log.error("Cube failed")
end

log.write("cube.mesh = " .. cube:get("Mesh"))
