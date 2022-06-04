/*
 * This example application connects via TCP to a modbus server at the specified address and port, and sends some
 * modbus requests to it.
 *
 * Since the platform for this example is linux, the platform arg is used to pass (to the linux file descriptor
 * read/write functions) a pointer to the file descriptor of our TCP connection
 *
 */

#include <stdio.h>

#include "nanomodbus.h"
#include "platform.h"


int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: client-tcp [address] [port]\n");
        return 1;
    }

    // Set up the TCP connection
    void* conn = connect_tcp(argv[1], argv[2]);
    if (!conn) {
        fprintf(stderr, "Error connecting to server\n");
        return 1;
    }

    nmbs_platform_conf platform_conf;
    platform_conf.transport = NMBS_TRANSPORT_TCP;
    platform_conf.read = read_fd_linux;
    platform_conf.write = write_fd_linux;
    platform_conf.arg = conn;    // Passing our TCP connection handle to the read/write functions

    // Create the modbus client
    nmbs_t nmbs;
    nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
    if (err != NMBS_ERROR_NONE) {
        fprintf(stderr, "Error creating modbus client\n");
        if (!nmbs_error_is_exception(err))
            return 1;
    }

    // Set only the response timeout. Byte timeout will be handled by the TCP connection
    nmbs_set_read_timeout(&nmbs, 1000);

    // Write 2 coils from address 64
    nmbs_bitfield coils;
    nmbs_bitfield_write(coils, 0, 1);
    nmbs_bitfield_write(coils, 1, 1);
    err = nmbs_write_multiple_coils(&nmbs, 64, 2, coils);
    if (err != NMBS_ERROR_NONE) {
        fprintf(stderr, "Error writing coils at address 64 - %s\n", nmbs_strerror(err));
        if (!nmbs_error_is_exception(err))
            return 1;
    }

    // Read 3 coils from address 64
    nmbs_bitfield_reset(coils);    // Reset whole bitfield to zero
    err = nmbs_read_coils(&nmbs, 64, 3, coils);
    if (err != NMBS_ERROR_NONE) {
        fprintf(stderr, "Error reading coils at address 64 - %s\n", nmbs_strerror(err));
        if (!nmbs_error_is_exception(err))
            return 1;
    }
    else {
        printf("Coil at address 64 value: %d\n", nmbs_bitfield_read(coils, 0));
        printf("Coil at address 65 value: %d\n", nmbs_bitfield_read(coils, 1));
        printf("Coil at address 66 value: %d\n", nmbs_bitfield_read(coils, 2));
    }

    // Write 2 holding registers at address 26
    uint16_t w_regs[2] = {123, 124};
    err = nmbs_write_multiple_registers(&nmbs, 26, 2, w_regs);
    if (err != NMBS_ERROR_NONE) {
        fprintf(stderr, "Error writing register at address 26 - %s", nmbs_strerror(err));
        if (!nmbs_error_is_exception(err))
            return 1;
    }

    // Read 2 holding registers from address 26
    uint16_t r_regs[2];
    err = nmbs_read_holding_registers(&nmbs, 26, 2, r_regs);
    if (err != NMBS_ERROR_NONE) {
        fprintf(stderr, "Error reading 2 holding registers at address 26 - %s\n", nmbs_strerror(err));
        if (!nmbs_error_is_exception(err))
            return 1;
    }
    else {
        printf("Register at address 26: %d\n", r_regs[0]);
        printf("Register at address 27: %d\n", r_regs[1]);
    }

    // Close the TCP connection
    disconnect(conn);

    // No need to destroy the nmbs instance, bye bye
    return 0;
}