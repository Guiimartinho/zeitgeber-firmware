#define C3_S1_WIDTH	16
#define C3_S1_HEIGHT	23
#define C3_S1_SIZE	(C3_S1_WIDTH * C3_S1_HEIGHT * 2)
uint16 __eds__ c3_s1_bytes[736] __attribute__((space(prog))) = {
	0x0000,0x0000,0x1082,0x4208,0x630c,0x8410,0x8410,0x8410,0x8410,0x8410,0x528a,0x1082,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x8410,0xffff,0xffff,0xffff,0xe71c,0xc618,0xc618,0xc618,0xf79e,0xffff,0xf79e,0x8410,0x0000,0x0000,0x0000,
	0x0000,0x4208,0x528a,0x2104,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x2104,0xb596,0xffff,0x738e,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xc618,0xf79e,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x630c,0xffff,0x3186,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x4208,0xffff,0x4208,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x528a,0xffff,0x2104,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xa514,0xffff,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x528a,0xffff,0x8410,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1082,0x8410,0xffff,0xd69a,0x1082,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0xc618,0xc618,0xc618,0xc618,0xd69a,0xffff,0xffff,0x9492,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x8410,0x8410,0x8410,0x8410,0x8410,0xb596,0xf79e,0xffff,0xb596,0x1082,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x630c,0xf79e,0xd69a,0x1082,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x528a,0xffff,0x738e,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xe71c,0xc618,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xc618,0xc618,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xc618,0xc618,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xc618,0xc618,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1082,0xffff,0xa514,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xb596,0xffff,0x4208,0x0000,
	0x0000,0x8410,0x8410,0x4208,0x3186,0x0000,0x0000,0x0000,0x0000,0x1082,0x528a,0xc618,0xffff,0xa514,0x0000,0x0000,
	0x0000,0xc618,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xe71c,0x738e,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x1082,0x4208,0x4208,0x630c,0x8410,0x8410,0x528a,0x3186,0x0000,0x0000,0x0000,0x0000,0x0000,
};
const image_t img_c3_s1 = {c3_s1_bytes, C3_S1_WIDTH, C3_S1_HEIGHT};
