/**
 * GPS Data Parser
 */
#include "nmea_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static double nmea_to_decimal(const char *nmea_coord) {
    double deg = atof(nmea_coord);
    int degrees = (int)(deg / 100);
    double minutes = deg - (degrees * 100);
    return degrees + (minutes / 60.0);
}

bool nmea_parse_line(const char *line, gps_data_t *data) {
    if (!line || line[0] != '$') return false;

    // Delete memset here to avoid overwriting previous data
    // memset(data, 0, sizeof(gps_data_t));

    char fields[20][24] = {0};
    int field_count = 0;
    const char *p = line;
    while (*p && field_count < 20) {
        int i = 0;
        while (*p && *p != ',' && *p != '*') {
            if (i < sizeof(fields[0]) - 1) {
                fields[field_count][i++] = *p;
            }
            p++;
        }
        fields[field_count][i] = '\0';
        if (*p == ',' || *p == '*') p++;
        field_count++;
    }

    if (strncmp(line, "$GPRMC", 6) == 0 || strncmp(line, "$GNRMC", 6) == 0) {
        if (fields[2][0] != 'A') {
            data->valid_fix = false;
            return false;
        }

        data->valid_fix = true;
        data->fix_quality = 1; // RMC frame does not have real quality, but we assume valid if there is fix
        sscanf(fields[1], "%2d%2d%2d", &data->hour, &data->minute, &data->second);
        data->latitude = nmea_to_decimal(fields[3]);
        data->lat_dir = fields[4][0];
        if (data->lat_dir == 'S') data->latitude *= -1;
        data->longitude = nmea_to_decimal(fields[5]);
        data->lon_dir = fields[6][0];
        if (data->lon_dir == 'W') data->longitude *= -1;
        sscanf(fields[9], "%2d%2d%2d", &data->day, &data->month, &data->year);
        data->year += 2000;
        return true;
    }

    if (strncmp(line, "$GPGGA", 6) == 0 || strncmp(line, "$GNGGA", 6) == 0) {
        // Only update if there are valid coordinates
        if (fields[2][0]) {
            data->latitude = nmea_to_decimal(fields[2]);
            data->lat_dir = fields[3][0];
            if (data->lat_dir == 'S') data->latitude *= -1;
        }

        if (fields[4][0]) {
            data->longitude = nmea_to_decimal(fields[4]);
            data->lon_dir = fields[5][0];
            if (data->lon_dir == 'W') data->longitude *= -1;
        }

        data->fix_quality = atoi(fields[6]);
        data->satellites = atoi(fields[7]);
        data->altitude = atof(fields[9]);

        return true;
    }

    return false;
}
