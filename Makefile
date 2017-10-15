all:
        g++ main.cpp -o modbus_device_scanner -I/usr/include/modbus -L/usr/lib lmodbus