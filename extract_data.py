import subprocess
import time
import struct

PAGE_SIZE = 256
FLASH_SIZE = 2 * 1024 * 1024
SECTOR_SIZE = 4096
LAST_SECTOR_START = FLASH_SIZE - SECTOR_SIZE

files = []

class pico_file:

    def clear_sizes(self):
        i=0
        while i < len(sizes):
            if sizes[i] == 0 or sizes[i] == 4294967295:
                sizes.pop(i)
            else:
                i += 1

    def __init__(self, name, start, overflow, sizes):
        self.name = name
        self.start = start
        self.overflow = overflow
        self.sizes = sizes
        self.end = 0
        self.clear_sizes()

    def add_end(self, end):
        self.end = end

    def add_data(self, data):
        self.data = data

    def return_name(self):
        return self.name

def picotool_flash_dump():
    subprocess.run(["picotool", "reboot", "-f", "-u"])
    time.sleep(10)
    subprocess.run(["picotool", "save", "-a", "-f", "flash_dump.bin"])

def read_and_decode(file, position: int, length: int) -> int:
    file.seek(position)
    raw_data = file.read(length)
    return struct.unpack('<I', raw_data)[0]

#picotool_flash_dump()

with open("flash_dump.bin", "rb") as f:

    for file_num in range(int(SECTOR_SIZE/PAGE_SIZE)):
        print(file_num)
        position = LAST_SECTOR_START + file_num * PAGE_SIZE

        start = read_and_decode(f, position, 4)
        print(start)
        position += 4
        for file in files:
            if file.return_name() == file_num-1:
                print("matching file name")
                file.add_end(start)
       
        overflow = read_and_decode(f, position, 4)
        position += 4

        sizes = []
        while position <= LAST_SECTOR_START + (file_num * PAGE_SIZE) + PAGE_SIZE/8:
            sizes.append(read_and_decode(f, position, 4))
            position += 4

        files.append(pico_file(file_num, start, overflow, sizes))


    for file in files:
        print(f"start: {file.start:#x}, end {file.end:#x}, overflow: {file.overflow:#x}, sizes: {file.sizes}")

    for i in range(0, 250*4, 4):
        print(f"i = {int(i/4)}")
        print(read_and_decode(f, 0x00024000 + i, 4))
