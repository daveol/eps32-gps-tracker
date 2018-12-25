#ifndef __NMEA_H__
#define __NMEA_H__

#include <stdbool.h>

/**
 * @brief Common NMEA prefixes for NMEA sentences
 */
typedef enum {
    NMEA_PREFIX_GP,
    NMEA_PREFIX_GN,
    NMEA_PREFIX_GL,
    NMEA_PREFIX_BD,
    NMEA_PREFIX_UNKNOWN
} nmea_prefix_t;

/**
 * @brief Common NMEA messages for NMEA sentences
 */
typedef enum {
    NMEA_MESSAGE_GSA,
    NMEA_MESSAGE_GSV,
    NMEA_MESSAGE_RMC,
    NMEA_MESSAGE_VTG,
    NMEA_MESSAGE_ZDA,
    NMEA_MESSAGE_TXT,
    NMEA_MESSAGE_UNKNOWN
} nmea_message_t;

/**
 * @brief Verify the checksum of an nmea sentence
 * 
 * @param sentence The nmea sentence, starting with '$' and ending with '\r\n'
 * @return true The checksum is valid
 * @return false The checksum is not valid or present
 */
bool nmea_validate_checksum(char* sentence);

/**
 * @brief Get the NMEA prefix type
 * 
 * @param sentence The nmea sentence, starting with '$' and ending with '\r\n'
 * @return nmea_prefix_t The prefix type
 */
nmea_prefix_t nmea_get_prefix(char* sentence);

/**
 * @brief Get the NMEA message type
 * 
 * @param sentence The nmea sentence, starting with '$' and ending with '\r\n'
 * @return nmea_message_t The message type
 */
nmea_message_t nmea_get_message(char* sentence);



#endif // __NMEA_H__