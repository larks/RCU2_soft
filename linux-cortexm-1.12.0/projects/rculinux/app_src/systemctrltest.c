/**
 * Test the system controller bits
 */

#include <stdio.h>
#include "sc_drivers/mss_sys_services/mss_sys_services.h"

/* private functions prototypes */
static void display_hex_values
(
    const uint8_t * in_buffer,
    uint32_t byte_length
);

int main()
{
    uint8_t serial_number[16];
    uint8_t user_code[4];
    uint8_t design_version[2];
    uint8_t device_certificate[512];
    uint8_t status;
    
    printf("Initialising system controller... \n");
    MSS_SYS_init(MSS_SYS_NO_EVENT_HANDLER);
    
    /*--------------------------------------------------------------------------
     * Device Serial Number (DSN).
     */
    status = MSS_SYS_get_serial_number(serial_number);
    
    if(MSS_SYS_SUCCESS == status)
    {
        printf("Device serial number: ");
        display_hex_values(serial_number, sizeof(serial_number));
    }
    
    /*--------------------------------------------------------------------------
     * User code.
     */
    status = MSS_SYS_get_user_code(user_code);
    if(MSS_SYS_SUCCESS == status)
    {
        printf("User code: ");
        display_hex_values(user_code, sizeof(user_code));
    }

    /*--------------------------------------------------------------------------
     * Design version.
     */
    status = MSS_SYS_get_design_version(design_version);
    if(MSS_SYS_SUCCESS == status)
    {
        printf("Design version: ");
        display_hex_values(design_version, sizeof(design_version));
    }
    
    /*--------------------------------------------------------------------------
     * Device certificate.
     */
    status = MSS_SYS_get_device_certificate(device_certificate);
    if(MSS_SYS_SUCCESS == status)
    {
        printf("Device certificate: ");
        display_hex_values(device_certificate, sizeof(device_certificate));
    }
    
    /*--------------------------------------------------------------------------
     * Check digest.
     */
    status = MSS_SYS_check_digest(MSS_SYS_DIGEST_CHECK_ENVM0);
    if(MSS_SYS_SUCCESS == status)
    {
        printf("Digest check success.");
    }
    else
    {
        uint8_t fabric_digest_check_failure;
        uint8_t envm0_digest_check_failure;
        uint8_t envm1_digest_check_failure;
        
        fabric_digest_check_failure = status & MSS_SYS_DIGEST_CHECK_FABRIC;
        envm0_digest_check_failure = status & MSS_SYS_DIGEST_CHECK_ENVM0;
        envm1_digest_check_failure = status & MSS_SYS_DIGEST_CHECK_ENVM1;
        
        printf("\r\nDigest check failure:");
        if(fabric_digest_check_failure)
        {
            printf("\r\n  Fabric digest check failed.");
        }
        if(envm0_digest_check_failure)
        {
            printf("\r\n  eNVM0 digest check failed.");
        }
        if(envm1_digest_check_failure)
        {
            printf("\r\n  eNVM1 digest check failed.");
        }
    }
    
    for(;;)
    {
        ;
    }
    
    return 0;
}

/*==============================================================================
  Display content of buffer passed as parameter as hex values
 */
static void display_hex_values
(
    const uint8_t * in_buffer,
    uint32_t byte_length
)
{
    uint8_t display_buffer[128];
    uint32_t inc;
    
    if(byte_length > 16u)
    {
        printf("\r\n");
    }
    
    for(inc = 0; inc < byte_length; ++inc)
    {
        if((inc > 1u) &&(0u == (inc % 16u)))
        {
            printf("\r\n" );
        }
        snprintf((char *)display_buffer, sizeof(display_buffer), "%02x ", in_buffer[inc]);
        printf("%s", display_buffer);
    }
}
