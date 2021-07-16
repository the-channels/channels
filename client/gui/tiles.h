
/* png2c.py 1.4.3
 *
 * tiles.png (64x64)
 * 8.0 x 8.0 (46 unique)
 *
 * base: 128
 */

#define TILES_BASE 128
#define TILES_LEN 46
static uchar tiles[] = {
0xd0, 0xd0, 0xd0, 0xd0, 0xdf, 0xc0, 0xff, 0xff, // y:0, x:0 (128)
0x0b, 0x0b, 0x0b, 0x0b, 0xfb, 0x03, 0xff, 0xff, // y:0, x:1 (129)
0xff, 0xff, 0x03, 0xfb, 0x0b, 0x0b, 0x0b, 0x0b, // y:0, x:2 (130)
0xff, 0xff, 0xc0, 0xdf, 0xd0, 0xd0, 0xd0, 0xd0, // y:0, x:3 (131)
0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, // y:0, x:4 (132)
0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, 0xd0, // y:0, x:5 (133)
0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, // y:0, x:6 (134)
0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, // y:0, x:7 (135)
0x00, 0x10, 0x00, 0x10, 0x0a, 0x00, 0x00, 0x00, // y:1, x:0 (136)
0x08, 0x00, 0x08, 0x00, 0xa8, 0x00, 0x00, 0x00, // y:1, x:1 (137)
0x00, 0x00, 0x00, 0x50, 0x08, 0x00, 0x08, 0x00, // y:1, x:2 (138)
0x00, 0x00, 0x00, 0x15, 0x00, 0x10, 0x00, 0x10, // y:1, x:3 (139)
0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, // y:1, x:4 (140)
0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, // y:1, x:5 (141)
0x00, 0x00, 0x00, 0x00, 0xaa, 0x00, 0x00, 0x00, // y:1, x:6 (142)
0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, // y:1, x:7 (143)
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:2, x:0 (144)
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:2, x:1 (145)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, // y:2, x:2 (146)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // y:2, x:3 (147)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, // y:2, x:4 (148)
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, // y:2, x:5 (149)
0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:2, x:6 (150)
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, // y:2, x:7 (151)
0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00, // y:3, x:0 (152)
0xff, 0xe9, 0xc9, 0x81, 0x81, 0xcf, 0xef, 0xff, // y:3, x:1 (153)
0xff, 0xff, 0xc3, 0xdf, 0xdf, 0xc3, 0xff, 0xff, // y:3, x:2 (154)
0x00, 0x03, 0x0f, 0x0c, 0x18, 0x18, 0x7e, 0x3c, // y:3, x:3 (155)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, // y:3, x:4 (156)
0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // y:3, x:5 (157)
0x3c, 0x7e, 0x18, 0x18, 0x30, 0xf0, 0xc0, 0x00, // y:3, x:6 (158)
0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, // y:3, x:7 (159)
0x00, 0x40, 0xc0, 0xf0, 0xfc, 0xcc, 0x46, 0x06, // y:4, x:0 (160)
0x60, 0x62, 0x33, 0x3f, 0x0f, 0x03, 0x02, 0x00, // y:4, x:1 (161)
0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, // y:4, x:2 (162)
0xff, 0xff, 0xff, 0xff, 0xff, 0xbd, 0x81, 0xff, // y:4, x:3 (163)
0x00, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x00, // y:4, x:4 (164)
0x00, 0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x00, // y:4, x:5 (165)
0xff, 0xff, 0xc3, 0xdb, 0xc3, 0xdf, 0xff, 0xff, // y:4, x:6 (166)
0x00, 0x00, 0x03, 0x07, 0x0e, 0x0c, 0x00, 0x00, // y:4, x:7 (167)
0x00, 0x00, 0xc0, 0xe0, 0x70, 0x30, 0x30, 0x70, // y:5, x:0 (168)
0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, // y:5, x:1 (169)
0xe0, 0xc0, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, // y:5, x:2 (170)
0xff, 0xe7, 0xdf, 0xc7, 0xdb, 0xdb, 0xe7, 0xff, // y:5, x:3 (171)
0xff, 0xc7, 0xfb, 0xfb, 0xf7, 0xef, 0xef, 0xff, // y:5, x:4 (172)
};
 