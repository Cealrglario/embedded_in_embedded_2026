'''
Constants & Configuration
'''

import psutil
import pynvml
import struct
import bleak
import time
import asyncio
import mmap

TARGET_DEVICE_NAME = "EiE 6248 Hardware Monitor" # Match the Zephyr config

# Map the C macros to Python string constants (format: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx")
CHAR_UUID_SERVICE = "01928374-1234-5678-1234-56789abcdef0"
CHAR_UUID_SCALAR = "01928374-1234-5678-1234-56789abcdef1"
CHAR_UUID_NETWORK = "01928374-1234-5678-1234-56789abcdef2"
CHAR_UUID_PERCENT = "01928374-1234-5678-1234-56789abcdef3"

# Define the shared memory name to obtain motherboard sensor data from HWiNFO64
shm_name = "Global\\HWiNFO_SENS_SM2"
shm_size = 1024*1024 # 1 MB is safely large enough to capture the header and all sensor readings.

# Initialize pynvml and grab the handle for the primary GPU which we want to read (GPU0)
pynvml.nvmlInit()
gpu_handle = pynvml.nvmlDeviceGetHandleByIndex(0)

# Global variables to keep track of network download and upload activity (since starting this script, NOT since computer boot-up)
last_net_bytes_recv = psutil.net_io_counters().bytes_recv
last_net_bytes_sent = psutil.net_io_counters().bytes_sent 
last_net_time = time.time()

'''
Hardware Acquisition Functions
'''

def read_hwinfo_shared_memory():
    # fileno is -1 for Windows named shared memory.
    try:
        shared_memory = mmap.mmap(-1, shm_size, shm_name, mmap.ACCESS_READ)
    except:
        print("\nError: HWiNFO shared memory not found.")
        print("Please ensure HWiNFO64 is running and 'Shared Memory Support' is enabled.")
        return None, None, None # Return three values as typically we'd return CPU clock, temperature and CPU package power
    
    # Parse the SM2 (shared memory) header
    # L = 32-bit unsigned int, Q = 64-bit unsigned int
    # We need to parse this header as it contains details of which exact bytes in this shared memory bytestream represent the sensor data we want

    # The header has the following format due to the C++ definition of the HWiNFO64 shared memory block as follows:
    '''
    typedef struct _HWiNFO_SENSORS_SHARED_MEM2 {
        DWORD  dwSignature;             // Magic number to verify HWiNFO
        DWORD  dwVersion;               // Shared memory version
        DWORD  dwRevision;              // Shared memory revision
        __int64 poll_time;              // Timestamp of the last sensor poll
        DWORD  dwOffsetOfSensorSection; // Memory offset where sensor names start
        DWORD  dwSizeOfSensorElement;   // Size in bytes of one sensor element
        DWORD  dwNumSensorElements;     // Total number of sensors
        DWORD  dwOffsetOfReadingSection;// Memory offset where actual sensor information starts
        DWORD  dwSizeOfReadingElement;  // Size in bytes of one reading element struct
        DWORD  dwNumReadingElements;    // Total number of sensors with values to read
    } HWiNFO_SENSORS_SHARED_MEM2;
    '''

    header_format = "<LLLQLLLLLL"
    header_size = struct.calcsize(header_format)
    
    shared_memory.seek(0) # Point to the start of the shared memory
    header_data = shared_memory.read(header_size) # Read the entire SM2 header
    if len(header_data) < header_size:
        return None, None, None # We may have read the wrong shared memory block somehow if we get here
    
    # Unpack the raw memory bytestream into distinct sections
    (signature, version, revision, poll_time, 
     offset_sensors, size_sensor, num_sensors, 
     offset_readings, size_reading, num_readings) = struct.unpack(header_format, header_data)
    
    # Validate that we're reading proper HWiNFO data
    if signature == 0x0:
        print("Cannot read HWiNFO shared memory region as it is empty.\n")
        return None, None, None
    
    # Define the Reading Element struct which is the struct WITHIN the shared memory block that contains our actual sensor data that we want to look through
    # 3x uint32 (12b), 2x char[128] (256b), 1x char[16] (16b), 4x double (32b) = 316 bytes total
    # The Reading Element struct holds information pertaining to each sensor and is defined as follows in C++, hence the following formatting:
    '''
    typedef struct _HWiNFO_SENSORS_READING_ELEMENT {
        SENSOR_READING_TYPE tReading; // Enum/DWORD: Type of sensor (Temp, Power, etc.)
        DWORD dwSensorIndex;          // The index of the parent sensor block
        DWORD dwReadingID;            // Unique ID for this specific reading
        char szLabelOrig[128];        // Original label string (e.g., "CPU Package")
        char szLabelUser[128];        // Custom label if renamed by the user
        char szUnit[16];              // Unit string (e.g., "°C", "W", "RPM")
        double Value;                 // Current sensor value
        double ValueMin;              // Minimum recorded value
        double ValueMax;              // Maximum recorded value
        double ValueAvg;              // Average recorded value
    } HWiNFO_SENSORS_READING_ELEMENT;
    '''
    reading_fmt = "<III128s128s16sdddd"
    unpack_size = struct.calcsize(reading_fmt)
    
    cpu_clock = 0 # Represents the average of all individual core clocks
    cpu_temp = 0
    cpu_power = 0 

    # To calculate average of all core clocks
    core_clocks = []

    # Loop through the Reading Element struct to find our CPU temp and package power readings
    for i in range(num_readings):
        shared_memory.seek(offset_readings + (i * size_reading)) # Start reading from the beginning of sensor values and read each value sequentially
        reading_data = shared_memory.read(size_reading) # Read the sensor value as a raw bytestream

        # Unpack the raw bytestream into distinct values up to the size of each sensor reading (calculated with the formatting)
        (tReading, sensor_idx, reading_id, label_orig_bytes, label_user_bytes, 
         unit_bytes, value, val_min, val_max, val_avg) = struct.unpack(reading_fmt, reading_data[:unpack_size]) 
        
        # Decode the original sensor label and strip the null terminators
        try:
            label_orig = label_orig_bytes.decode('utf-8', errors='ignore').split('\x00')[0]
        except:
            continue

        # Filter for CPU clock, temperature and package power
        if tReading == 1: # If reading is from a temperature sensor
            print(f'TEMPERATURE LABEL FOUND: [{label_orig}]')
            if label_orig in ["CPU Package", "CPU (Tctl/Tdie)"]: # CPU Package for Intel, CPU (Tctl/Tdie) for AMD
                cpu_temp = value
        elif tReading == 5: # If reading is from a power draw sensor
            print(f'POWER LABEL FOUND: [{label_orig}]')
            if label_orig == "CPU Package Power":
                cpu_power = value
        elif tReading == 6: # If reading is from a core clock sensor
            print(f'CLOCK LABEL FOUND: [{label_orig}]')

            # We want to display the average of the individual core clocks that we read
            if ("P-core" in label_orig or "E-core" in label_orig) and "Clock" in label_orig and "Effective" not in label_orig:
                core_clocks.append(value)

    # Always clean up the memory map mapping
    shared_memory.close()

    for value in core_clocks:
        cpu_clock += value

    cpu_clock = cpu_clock / len(core_clocks) # Get average core clock

    return cpu_clock, cpu_temp, cpu_power


def get_scalar_metrics():
    ## Use HWiNFO64 shared memory for CPU Clock/Temp/Power
    ## Use pynvml to get GPU Temp
    cpu_clock, cpu_temp, cpu_power = read_hwinfo_shared_memory() # Retrieve instantaneous CPU temperature and package power
    
    # If CPU temp/power is None, then handle it and notify user to run HWiNFO with shared memory support
    if (cpu_clock == None) or (cpu_temp == None) or (cpu_power == None):
        cpu_clock = 0
        cpu_temp = 0
        cpu_power = 0
        print("Failed to access HWiNFO shared memory.\n")

    gpu_temp = pynvml.nvmlDeviceGetTemperature(gpu_handle, pynvml.NVML_TEMPERATURE_GPU) # Retrieve instantaneous GPU temperature

    print(f'\nRetrieved scalar metrics: CPU Clock ({cpu_clock} MHz), CPU Power ({cpu_power} W), CPU Temp ({cpu_temp}°C), GPU Temp ({gpu_temp}°C)')
    return int(cpu_clock), int(cpu_power), int(cpu_temp), int(gpu_temp)

def get_network_metrics():
    ## Use psutil.net_io_counters() to get bytes sent/recv
    ## Convert to bits
    global last_net_bytes_recv
    global last_net_bytes_sent
    global last_net_time

    current_time = time.time()
    time_since_last = current_time - last_net_time

    current_bytes_recv_since_last = psutil.net_io_counters().bytes_recv - last_net_bytes_recv
    current_bytes_sent_since_last = psutil.net_io_counters().bytes_sent - last_net_bytes_sent
    
    # The current time and bytes will be the last time and bytes in the next iteration
    last_net_bytes_recv = psutil.net_io_counters().bytes_recv
    last_net_bytes_sent = psutil.net_io_counters().bytes_sent
    last_net_time = current_time

    down_bits = (current_bytes_recv_since_last / time_since_last) / (1e3) # Get average download network activity in Kb/s since last iteration
    up_bits = (current_bytes_sent_since_last / time_since_last) / (1e3) # Get average upload network activity in Kb/s since last iteration

    print(f'Retrieved network metrics: Network Download ({down_bits} Kb/s), Network Upload ({up_bits} Kb/s)')
    return int(down_bits), int(up_bits)

def get_percentage_metrics():
    ## Use psutil for CPU % and RAM usage (convert bytes to GB)
    ## Use pynvml for GPU %
    cpu_percent = psutil.cpu_percent(interval=None) # Retrieve instantaneous CPU usage percentage
    gpu_util = pynvml.nvmlDeviceGetUtilizationRates(gpu_handle) # Retrieve instantaneous GPU usage percentage
    gpu_percent = gpu_util.gpu

    ram_usage_percent = psutil.virtual_memory().percent # Retrieve instantaneous RAM usage in percent

    print(f'Received percentage metrics: CPU Percent ({cpu_percent}%), GPU Percent ({gpu_percent}%), RAM Percent ({ram_usage_percent}%)')
    return int(cpu_percent), int(gpu_percent), int(ram_usage_percent)
     
'''
Data Serialization (The "struct" module)
'''
    # Use Python's struct.pack() here. 
    # Your C structs use uint32_t. In Python's struct format characters:
    # '<' means Little-Endian.
    # 'I' means unsigned 32-bit integer.

def pack_metrics_to_bytes(metric_type, data_array):
    # NOTE: the '*' operator before the data_array parameter deconstructs a tuple into individual elements, which is required for struct.pack() to work
    if metric_type == "scalar":
        return struct.pack("<IIII", *data_array) # 4 integers = 16 bytes
    if metric_type == "network":
        return struct.pack("<II", *data_array)   # 2 integers = 8 bytes
    if metric_type == "percent":
        return struct.pack("<III", *data_array)  # 3 integers = 12 bytes
    else:
        raise ValueError("Unknown metric type")

'''
Asynchronous BLE Main Loop
'''
async def run_ble_client():
    # Step 1: Scan for the nRF52840
    device = await bleak.BleakScanner.find_device_by_name(TARGET_DEVICE_NAME)
    
    if device == None:
        print("Device not found.")
        return

    # Step 2: Establish Connection
    async with bleak.BleakClient(device) as client:
        print("Connected!")
        
        # Step 3: The infinite transmission loop
        while True:
            # Gather metrics
            scalar_data = get_scalar_metrics()
            network_data = get_network_metrics()
            percent_data = get_percentage_metrics()
            
            # Pack metrics
            scalar_bytes = pack_metrics_to_bytes("scalar", scalar_data)
            network_bytes = pack_metrics_to_bytes("network", network_data)
            percent_bytes = pack_metrics_to_bytes("percent", percent_data)
            
            # Send to nRF52840
            # Use Write Without Response to match Zephyr BT_GATT_CHRC_WRITE_WITHOUT_RESP
            await client.write_gatt_char(CHAR_UUID_SCALAR, scalar_bytes, response=False)
            await client.write_gatt_char(CHAR_UUID_PERCENT, percent_bytes, response=False)
            await client.write_gatt_char(CHAR_UUID_NETWORK, network_bytes, response=False)
            
            # Send the metrics every 2 seconds so we don't overwhelm the BLE connection
            await asyncio.sleep(2)

if __name__ == "__main__":
    asyncio.run(run_ble_client())