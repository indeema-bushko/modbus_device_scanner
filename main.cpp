#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <modbus/modbus.h>

int main() {

    struct timeval response_timeout;
    struct timeval new_response_timeout;

    for (std::string port_name: {"/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2"}) {
        std::cout << "Scanning port -> " << port_name << std::endl;

        for (int speed: {9600, 19200, 115200}) {

            for (char parity: {'N', 'E', 'O'}) {

                const int data_bits = 8;
                const int stop_bits = (parity == 'N') ? 2 : 1;

                modbus_t *context = modbus_new_rtu(port_name.c_str(), speed, parity, data_bits, stop_bits);

                if (!context) {
                    std::cout << "Failed to create modbus context: error -> " << modbus_strerror(errno) << std::endl;
		    continue;	
                }

                if (modbus_connect(context) == -1) {
                    std::cout << "Unable to connect: error -> " << modbus_strerror(errno) << std::endl;
                    modbus_free(context);
                    continue;
                }

                modbus_get_response_timeout(context, &response_timeout);

                new_response_timeout.tv_sec = 0;
                new_response_timeout.tv_usec = 100000;

                modbus_set_response_timeout(context, &new_response_timeout);

                std::cout << "Successfully connected at port: " << port_name << ", speed: " << speed
                          << ", parity: " << parity << ", stop_bits: " << stop_bits << std::endl;

                uint16_t tab_request[64];
		const int read_bytes = 10;
		const int address = 0;

                for (int i = 1; i <= 247; i++) {
                    modbus_set_slave(context, i);
                    int read_count = modbus_read_registers(context, address, read_bytes, tab_request);
                    if (read_count == -1) {
                        if (errno != 110) {
                            std::cout << "Error: " << modbus_strerror(errno);
                            return (-2);
                        }
                        continue;
                    }
                    std::cout << "Read " << read_bytes << " bytes from address " 
				<< address << " of holding register: read count -> " << read_count << std::endl;
                    for (int j = 0; j < read_count; j++) {
                        printf("reg[%d]=%d (0x%X)\n", j, tab_request[j], tab_request[j]);
                    }
                    std::cout << "Success found device with ID: " << i << std::endl;
                }

                modbus_close(context);
                modbus_free(context);
            }
        }
    }

    return 0;
}
