#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include <stdbool.h>

typedef struct {
    bool valid_fix;
    double latitude;
    double longitude;
    char lat_dir;
    char lon_dir;
    int hour, minute, second;
    int day, month, year;
    int satellites;
    double altitude;
    int fix_quality;
} gps_data_t;

bool nmea_parse_line(const char *line, gps_data_t *data);

#endif
