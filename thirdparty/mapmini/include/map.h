#include <stdio.h>
#include <stdint.h>

typedef struct _mapsforge_zoom_interval {
    uint8_t base_zoom;
    uint8_t max_zoom;
    uint8_t min_zoom;
    uint64_t sub_file;
    uint64_t sub_file_size;
    uint16_t n_tiles_x;
    uint16_t n_tiles_y;
} mapsforge_zoom_interval;

typedef struct _mapsforge_file_header {
    uint32_t header_size;
    uint32_t file_version;
    uint64_t file_size; 
    uint64_t file_creation; // Milliseconds!
    int32_t bounding_box[4];
    uint16_t tile_size;
    char projection[16];
    uint8_t flags;
    int32_t init_lat_long[2];
    uint8_t init_zoom;
    char lang_pref[32];
    char comment[40];
    char created_by[40];
    uint16_t n_poi_tags;
    char poi_tag_names[100][32];
    uint16_t n_way_tags;
    char way_tag_names[360][32];
    uint8_t n_zoom_intervals;
    mapsforge_zoom_interval zoom_conf[3];
} mapsforge_file_header;

typedef struct { // Storage for subtile quadrants
    uint16_t subtile_q[4];
} subtile_q_maps;

int load_map(char* filename, uint32_t x_in, uint32_t y_in, uint32_t z_in, int16_t x0, int16_t y0, uint16_t st, float rot, float size);
int long2tilex(double lon, int z);
int lat2tiley(double lat, int z);
float tilex2long(int x, int z);
float tiley2lat(int y, int z);
subtile_q_maps get_st(int16_t xo, int16_t yo, int16_t size);
