archive = require("lzip")

archive.compress_files("Example_one.zip", {"File_One.txt", "File_Two.txt", "File_Three.txt"}, ZIP_DEFAULT_COMPRESSION_LEVEL)

