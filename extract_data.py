import subprocess
import time
import struct
import os
import csv
import statistics

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
        self.data = {}

        self.file_is_oveflow = False
        if position > self.end:
            self.file_is_oveflow = True
        
        self.clear_sizes()

    def add_end(self, end):
        self.end = end

    def add_data(self, data):
        self.data = data

    def return_name(self):
        return self.name
    
    def read_file(self, file):
        position = self.start
        empty_counter = 0
        last_pid = 0
        last_time = 0

        while position <= self.end or self.file_is_oveflow:

            if position >= LAST_SECTOR_START:
                position = self.overflow
                self.file_is_oveflow = False

            for length in self.sizes:


                if len(self.data) ==  6:
                    position = 0x50700

                data_bit = read_and_decode(file, position, length)

                if empty_counter  == 5*len(self.sizes):
                    return
                elif data_bit == 0 or data_bit == 0xffffffff or data_bit == 0xffff or data_bit == -1:
                    empty_counter += 1
                else:
                    empty_counter = 0

                #aplication specific
                if length == 1:

                    last_pid = data_bit

                    #fix bug in c code
                    if data_bit not in self.data:
                        if  len(self.data) < 5:
                            self.data[data_bit] = {}
                        else:
                            print(f"Problem at {position:#x}, seeking known PID")
                            original_position = position

                            for i in range(8):
                                
                                check_position = original_position - 4 + i
                                                    
                                data_bit = read_and_decode(file, check_position, 1)
                                
                                if data_bit in self.data.keys():
                                    print(f"Found known PID {data_bit} at position {check_position:#x}")
                                    
                                    timestamp_position = check_position - 4
                                    last_time = read_and_decode(file, timestamp_position, 4)
                                    position = check_position  # Update position to the valid location found
                                    break
                            else:  
                                print(f"No known PID found after checking 8 positions, skipping another 8")
                                position += 8

                    
                elif length == 4:
                    last_time = data_bit
                elif length ==2:
                    if last_pid in self.data:
                        self.data[last_pid][last_time] = data_bit

                if len(self.data) ==  6:
                    None
                
                position = position + length

    def delete_outliers(self):
        for pid in self.data:
            values = list(self.data[pid].values())
        
            # Calculate mean and standard deviation
            mean = statistics.mean(values)
            stdev = statistics.stdev(values) if len(values) > 1 else 0
            
            # Filter dictionary to remove outliers
            filtered_dict = {k: v for k, v in self.data[pid].items() 
                            if abs(v - mean) <= 2 * stdev}
            
            self.data[pid] = filtered_dict

    def write_csv(self):
        all_keys = set()
        for inner_dict in self.data.values():
            all_keys.update(inner_dict.keys())
        
        # Convert to sorted list for consistent column order
        all_keys = sorted(all_keys)
        
        with open("data/can_log.csv", 'w', newline='') as csvfile:
            fieldnames = ["PID"] + all_keys
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            
            for pid, inner_dict in self.data.items():
                row = {"PID": pid}
                # Add all available values, leave missing ones empty
                for key in all_keys:
                    row[key] = inner_dict.get(key, '')
                writer.writerow(row)

        

    


def picotool_flash_dump():
    subprocess.run(["picotool", "reboot", "-f", "-u"])
    time.sleep(10)

    subprocess.run(["picotool", "save", "-a", "-f", r"data/flash_dump.bin"])

def read_and_decode(file, position: int, length: int) -> int:
    file.seek(position)
    raw_data = file.read(length)
    if length == 4:
        return struct.unpack('<I', raw_data)[0]
    elif length == 2:
        return struct.unpack('<h', raw_data)[0]
    elif length == 1:
        return struct.unpack('<B', raw_data)[0]



if not os.path.exists(r"data"):
    os.makedirs(r"data")

#picotool_flash_dump()


with open(r"data/flash_dump.bin", "rb") as f:

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
       
        if start == 0xffffffff:
            break
        overflow = read_and_decode(f, position, 4)
        position += 4

        sizes = []
        while position <= LAST_SECTOR_START + (file_num * PAGE_SIZE) + PAGE_SIZE/8:
            sizes.append(read_and_decode(f, position, 4))
            position += 4

        files.append(pico_file(file_num, start, overflow, sizes))


    for file in files:
        print(f"File start: {file.start:#x}, end {file.end:#x}, overflow: {file.overflow:#x}, sizes: {file.sizes}")


    files[8].read_file(f)
    files[8].delete_outliers()
    files[8].write_csv()
    print(len(files[8].data))
    print(file.data)


