log.write("Test script")

cube = meshproc._createCommand("generator.Cube");

if cube:invoke() then
	log.write("Cube generated")
else
	log.error("Cube failed")
end

log.write("end of script.")
