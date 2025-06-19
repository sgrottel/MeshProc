log.write("Test script")

cube = meshproc._createCommand("generator.Cube");

io.write("mo = ", cube:get(), "\n")
cube:set(12)
io.write("mo = ", cube:get(), "\n")
print(cube)

cube = nil

collectgarbage()

log.write(nil)

log.write("end of script.")
