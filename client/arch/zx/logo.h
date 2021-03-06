
/* png2c.py 1.4.3
 *
 * logo.png (96x32)
 * 12.0 x 4.0 (38 unique)
 *
 * base: 128
 */

#define TILES_BASE 128
#define TILES_LEN 38
static uchar tiles[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:0 (128)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:1 (129)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:2 (129)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:3 (129)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:4 (129)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:5 (129)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:6 (129)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:0, x:7 (129)
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, // y:0, x:8 (129)
0x00, 0x00, 0x07, 0x3f, 0xff, 0xff, 0xff, 0xff, // y:0, x:9 (130)
0x00, 0x00, 0xf8, 0xff, 0xff, 0xf3, 0xf1, 0xf0, // y:0, x:10 (131)
0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, // y:0, x:11 (132)
0x00, 0x1f, 0x3f, 0x7f, 0x79, 0xf0, 0xf0, 0xf0, // y:1, x:0 (133)
0x00, 0x87, 0xc7, 0xe7, 0xe7, 0xe7, 0xe7, 0xf7, // y:1, x:1 (134)
0x00, 0x9e, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, // y:1, x:2 (135)
0x00, 0x00, 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, // y:1, x:3 (136)
0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, // y:1, x:4 (137)
0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, // y:1, x:5 (138)
0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, // y:1, x:6 (138)
0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, // y:1, x:7 (138)
0x0f, 0x00, 0xe0, 0xe0, 0xe0, 0x00, 0x1f, 0x1f, // y:1, x:8 (138)
0x00, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, // y:1, x:9 (139)
0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, // y:1, x:10 (140)
0x7c, 0x3c, 0x1e, 0x0e, 0x0e, 0x1f, 0x3f, 0x7f, // y:1, x:11 (141)
0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, // y:2, x:0 (142)
0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, // y:2, x:1 (143)
0x03, 0x03, 0x03, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, // y:2, x:2 (144)
0x1f, 0x3f, 0x7f, 0x71, 0x00, 0x00, 0x0f, 0x3f, // y:2, x:3 (145)
0x70, 0x38, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, // y:2, x:4 (146)
0x3f, 0x0f, 0x07, 0xc7, 0xe3, 0xe3, 0xe3, 0xe3, // y:2, x:5 (147)
0x03, 0x80, 0x80, 0x8c, 0x8e, 0x8e, 0x8e, 0x8e, // y:2, x:6 (148)
0x01, 0x03, 0x87, 0x87, 0xc7, 0xc7, 0xc7, 0xc7, // y:2, x:7 (149)
0x07, 0x03, 0x03, 0x61, 0xf1, 0xf1, 0xf1, 0x01, // y:2, x:8 (150)
0x70, 0x70, 0x71, 0x71, 0x71, 0x71, 0x71, 0x70, // y:2, x:9 (151)
0x7e, 0xfe, 0xff, 0xc7, 0xc0, 0xe0, 0xf8, 0xfe, // y:2, x:10 (152)
0x00, 0x80, 0xc0, 0xc0, 0x00, 0x01, 0x01, 0x81, // y:2, x:11 (153)
0x0f, 0x0f, 0x0f, 0x0f, 0x8e, 0x80, 0xc0, 0xe0, // y:3, x:0 (154)
0x07, 0xf7, 0xe7, 0xe7, 0xe7, 0xc7, 0xc7, 0x87, // y:3, x:1 (155)
0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x9e, // y:3, x:2 (156)
0x7f, 0x78, 0x70, 0x70, 0x71, 0x7f, 0x7e, 0x3c, // y:3, x:3 (157)
0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, // y:3, x:4 (158)
0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1e, 0x1e, // y:3, x:5 (159)
0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, // y:3, x:6 (160)
0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x1c, 0x1e, // y:3, x:7 (161)
0x01, 0xff, 0xff, 0xe3, 0x61, 0x03, 0x03, 0x07, // y:3, x:8 (162)
0x70, 0x70, 0x70, 0x71, 0x79, 0x7d, 0x3c, 0x1c, // y:3, x:9 (163)
0x3f, 0x0f, 0x07, 0xc7, 0xc7, 0xff, 0xfe, 0x7c, // y:3, x:10 (164)
0x3c, 0x3c, 0x38, 0x30, 0x20, 0x00, 0x00, 0x00, // y:3, x:11 (165)
};
 
static uchar tile_colors[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x43, 0x43, 0x43, 0x43, 0x47, 0x47, 0x47, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x46, 0x58, 0x43, 0x78, 0x47, 0x78, 0x46, 0x70, 0x70, 0x70, 0x46, 0x70, 0x46, 0x46, 0x58, 0x78, 0x47, 0x47, 0x46, 0x70, 0x46, 0x46, 0x70, 0x70, 0x46, 0x46, 0x43};
