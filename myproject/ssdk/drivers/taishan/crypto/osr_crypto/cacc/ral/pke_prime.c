/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <stdio.h>
#include "utility.h"
#include "pke.h"
#include "pke_prime.h"
#include "trng.h"

/* just for internal test */

#ifdef PRIME_TEST
volatile uint32_t xxxx = 0, xxxx2 = 0, count = 0;
#endif

/* small prime number table */
const uint16_t primetable[PTL_MAX] = {
    3,      5,      7,      11,     13,     17,     19,     23,     29,     31,
    37,     41,     43,     47,     53,     59,     61,     67,     71,     73,
    79,     83,     89,     97,     101,    103,    107,    109,    113,    127,
    131,    137,    139,    149,    151,    157,    163,    167,    173,    179,
    181,    191,    193,    197,    199,    211,    223,    227,    229,    233,    /* 50 */
    239,    241,    251,    257,    263,    269,    271,    277,    281,    283,
    293,    307,    311,    313,    317,    331,    337,    347,    349,    353,
    359,    367,    373,    379,    383,    389,    397,    401,    409,    419,
    421,    431,    433,    439,    443,    449,    457,    461,    463,    467,
    479,    487,    491,    499,    503,    509,    521,    523,    541,    547,    /* 100 */
    557,    563,    569,    571,    577,    587,    593,    599,    601,    607,
    613,    617,    619,    631,    641,    643,    647,    653,    659,    661,
    673,    677,    683,    691,    701,    709,    719,    727,    733,    739,
    743,    751,    757,    761,    769,    773,    787,    797,    809,    811,
    821,    823,    827,    829,    839,    853,    857,    859,    863,    877,    /* 150 */
    881,    883,    887,    907,    911,    919,    929,    937,    941,    947,
    953,    967,    971,    977,    983,    991,    997,    1009,   1013,   1019,
    1021,   1031,   1033,   1039,   1049,   1051,   1061,   1063,   1069,   1087,
    1091,   1093,   1097,   1103,   1109,   1117,   1123,   1129,   1151,   1153,
    1163,   1171,   1181,   1187,   1193,   1201,   1213,   1217,   1223,   1229,   /* 200 */
    1231,   1237,   1249,   1259,   1277,   1279,   1283,   1289,   1291,   1297,
    1301,   1303,   1307,   1319,   1321,   1327,   1361,   1367,   1373,   1381,
    1399,   1409,   1423,   1427,   1429,   1433,   1439,   1447,   1451,   1453,
    1459,   1471,   1481,   1483,   1487,   1489,   1493,   1499,   1511,   1523,
    1531,   1543,   1549,   1553,   1559,   1567,   1571,   1579,   1583,   1597,   /* 250 */
    1601,   1607,   1609,   1613,   1619,   1621,   1627,   1637,   1657,   1663,
    1667,   1669,   1693,   1697,   1699,   1709,   1721,   1723,   1733,   1741,
    1747,   1753,   1759,   1777,   1783,   1787,   1789,   1801,   1811,   1823,
    1831,   1847,   1861,   1867,   1871,   1873,   1877,   1879,   1889,   1901,
    1907,   1913,   1931,   1933,   1949,   1951,   1973,   1979,   1987,   1993,   /* 300 */
    1997,   1999,   2003,   2011,   2017,   2027,   2029,   2039,   2053,   2063,
    2069,   2081,   2083,   2087,   2089,   2099,   2111,   2113,   2129,   2131,
    2137,   2141,   2143,   2153,   2161,   2179,   2203,   2207,   2213,   2221,
    2237,   2239,   2243,   2251,   2267,   2269,   2273,   2281,   2287,   2293,
    2297,   2309,   2311,   2333,   2339,   2341,   2347,   2351,   2357,   2371,   /* 350 */
    2377,   2381,   2383,   2389,   2393,   2399,   2411,   2417,   2423,   2437,
    2441,   2447,   2459,   2467,   2473,   2477,   2503,   2521,   2531,   2539,
    2543,   2549,   2551,   2557,   2579,   2591,   2593,   2609,   2617,   2621,
    2633,   2647,   2657,   2659,   2663,   2671,   2677,   2683,   2687,   2689,
    2693,   2699,   2707,   2711,   2713,   2719,   2729,   2731,   2741,   2749,   /* 400 */
};

#if(BIGINT_DIV_CHOICE == 1)
/* 0xFFFFFFFFFFFFFFFF/prime  */
const double_uint32_t primetable_s[PTL_MAX] = {
    {0x55555555, 0x55555555}, {0x33333333, 0x33333333},
    {0x92492492, 0x24924924}, {0x5d1745d1, 0x1745d174},
    {0xb13b13b1, 0x13b13b13}, {0x0f0f0f0f, 0x0f0f0f0f},
    {0x50d79435, 0x0d79435e}, {0x8590b216, 0x0b21642c},
    {0x8d3dcb08, 0x08d3dcb0}, {0x21084210, 0x08421084},
    {0x306eb3e4, 0x06eb3e45}, {0xe7063e70, 0x063e7063},
    {0x5f417d05, 0x05f417d0}, {0xe4c415c9, 0x0572620a},
    {0xcade304d, 0x04d4873e}, {0xdd49c341, 0x0456c797},
    {0xef368eb0, 0x04325c53}, {0x7e16ece5, 0x03d22635},
    {0x2073615a, 0x039b0ad1}, {0x70381c0e, 0x0381c0e0},
    {0xa2067b23, 0x033d91d2}, {0xed7e7534, 0x03159721},
    {0x81702e05, 0x02e05c0b}, {0x5c5f02a3, 0x02a3a0fd},
    {0xac5b3f5d, 0x0288df0c}, {0x9c95204f, 0x027c4597},
    {0x456217ec, 0x02647c69}, {0xb02593f6, 0x02593f69},
    {0x243f6f02, 0x0243f6f0}, {0x20408102, 0x02040810},
    {0xe4a42715, 0x01f44659}, {0x3f8868a4, 0x01de5d6e},
    {0x4b82c339, 0x01d77b65}, {0xdda338b2, 0x01b7d6c3},
    {0x06c80d90, 0x01b20364}, {0x97a4b01a, 0x01a16d3f},
    {0x9d0e228d, 0x01920fb4}, {0x0abb0499, 0x01886e5f},
    {0x8e0ecc35, 0x017ad220}, {0xb4337c6c, 0x016e1f76},
    {0x15372904, 0x016a13cd}, {0xc506b39a, 0x01571ed3},
    {0x8f40feac, 0x01539094}, {0x725af6e7, 0x014cab88},
    {0x3b2d066e, 0x0149539e}, {0x3de07479, 0x013698df},
    {0x08092f11, 0x0125e227}, {0xc67c0d88, 0x0120b470},
    {0xb3fb8744, 0x011e2ef3}, {0x08ca29c0, 0x01194538},  /* 50 */
    {0x75d30336, 0x0112358e}, {0x0fef010f, 0x010fef01},
    {0x7d734041, 0x0105197f}, {0x00ff00ff, 0x00ff00ff},
    {0x211855a8, 0x00f92fb2}, {0x2cba8723, 0x00f3a0d5},
    {0xcee0d399, 0x00f1d48b}, {0x18f3fc4d, 0x00ec9791},
    {0x1fe2d8d3, 0x00e93965}, {0xe225fe30, 0x00e79372},
    {0x74346c57, 0x00dfac1f}, {0x7c3f5fe5, 0x00d578e9},
    {0x3b445250, 0x00d2ba08}, {0x3e28e502, 0x00d16154},
    {0xbb5b4169, 0x00cebcf8}, {0x0317f9d0, 0x00c5fe74},
    {0x13c0309e, 0x00c27806}, {0x5db1cc5b, 0x00bcdd53},
    {0x8cd63069, 0x00bbc840}, {0x2a0ff465, 0x00b9a786},
    {0x340e4307, 0x00b68d31}, {0x29da5519, 0x00b2927c},
    {0xa1496fdf, 0x00afb321}, {0x891e6551, 0x00aceb0f},
    {0xd3e2970f, 0x00ab1cbd}, {0x088e262b, 0x00a87917},
    {0x6bb00a51, 0x00a513fd}, {0xa2cb0331, 0x00a36e71},
    {0x88732b30, 0x00a03c16}, {0x9b30446d, 0x009c6916},
    {0x8e4a2f6e, 0x009baade}, {0x56201301, 0x00980e41},
    {0x0ff68a58, 0x00975a75}, {0x979e0829, 0x009548e4},
    {0xc50e726b, 0x0093efd1}, {0xb8bb02d9, 0x0091f5bc},
    {0xe3fdc261, 0x008f67a1}, {0xe0e702c6, 0x008e2917},
    {0x3f95d715, 0x008d8be3}, {0x1c815ed5, 0x008c5584},
    {0xcd3a4133, 0x0088d180}, {0xb1acf1ce, 0x00869222},
    {0x917765ab, 0x0085797b}, {0xe3c897db, 0x008355ac},
    {0x60b3262b, 0x00824a4e}, {0xb28bd1ba, 0x0080c121},
    {0x397d4c29, 0x007dc9f3}, {0x8fe88139, 0x007d4ece},
    {0x65bcce50, 0x0079237d}, {0xc5f7936c, 0x0077cf53},  /* 100 */
    {0xcfbdd11e, 0x0075a8ac}, {0x557c228e, 0x007467ac},
    {0xed8db8e9, 0x00732d70}, {0x24c3797f, 0x0072c62a},
    {0x7f55a10d, 0x007194a1}, {0xb41da7e7, 0x006fa549},
    {0xe6f61221, 0x006e8419}, {0x356c207b, 0x006d68b5},
    {0x3685c01b, 0x006d0b80}, {0xa8b2d207, 0x006bf790},
    {0xef4b96c2, 0x006ae907}, {0x1a23aead, 0x006a3799},
    {0xd4295b66, 0x0069dfbd}, {0x45c8033e, 0x0067dc4c},
    {0xff99c27f, 0x00663d80}, {0xe3559948, 0x0065ec17},
    {0x35cfba5c, 0x00654ac8}, {0x4ae10772, 0x00645c85},
    {0x0e5f901f, 0x00637299}, {0x3c07beef, 0x00632591},
    {0x9e9f0061, 0x006160ff}, {0x20e5e88e, 0x0060cdb5},
    {0x7fd005ff, 0x005ff401}, {0x31a4dccd, 0x005ed79e},
    {0xd48ac5ef, 0x005d7d42}, {0xccba5028, 0x005c6f35},
    {0xec6ad0a5, 0x005b2618}, {0x748e42e7, 0x005a2553},
    {0xf744cd5b, 0x0059686c}, {0xbab79976, 0x0058ae97},
    {0x1876865f, 0x0058345f}, {0xbb24795a, 0x005743d5},
    {0xd1ab74ab, 0x005692c4}, {0xa4d5f337, 0x00561e46},
    {0x06533997, 0x005538ed}, {0xf2c0bec2, 0x0054c807},
    {0xbc572d36, 0x005345ef}, {0x8f941345, 0x00523a75},
    {0x0f816c89, 0x00510237}, {0x9fb94acf, 0x0050cf12},
    {0x41cafdd1, 0x004fd319}, {0x4aa75945, 0x004fa170},
    {0xd45a63ad, 0x004f3ed6}, {0x7154ebed, 0x004f0de5},
    {0x8815f811, 0x004e1cae}, {0xa5f6ff19, 0x004cd47b},
    {0x734df709, 0x004c78ae}, {0xed85cfb8, 0x004c4b19},
    {0x221d1218, 0x004bf093}, {0x21dc633f, 0x004aba3c},  /* 150 */
    {0xc344de00, 0x004a6360}, {0x9f74d68a, 0x004a383e},
    {0xbabb9940, 0x0049e28f}, {0x57c78cd7, 0x0048417b},
    {0x713f3a2b, 0x0047f043}, {0xa10281cf, 0x00474ff2},
    {0x9a978f91, 0x00468b6f}, {0x1caff2e2, 0x0045f13f},
    {0x8cec23e9, 0x0045a522}, {0x556c66b9, 0x0045342c},
    {0x3feeced7, 0x0044c4a2}, {0x0d3c9fe6, 0x0043c5c2},
    {0x4b239798, 0x00437e49}, {0x118e47cb, 0x0043142d},
    {0x73a13458, 0x0042ab5c}, {0x0db0f3db, 0x00422195},
    {0xf80a4553, 0x0041bbb2}, {0x612c6680, 0x0040f391},
    {0x4173fefd, 0x0040b1e9}, {0x7d9d0445, 0x00405064},
    {0x1b144f3b, 0x00403024}, {0xab542cb1, 0x003f90c2},
    {0x2d59f597, 0x003f7141}, {0x01b98841, 0x003f1377},
    {0x6b60e278, 0x003e7988}, {0x16a7181d, 0x003e5b19},
    {0x0968f524, 0x003dc4a5}, {0xc9550321, 0x003da6e4},
    {0x06f1def3, 0x003d4e4f}, {0xdd24f9a4, 0x003c4a6b},
    {0x4b525c73, 0x003c11d5}, {0xc5721065, 0x003bf5b1},
    {0x862f23b4, 0x003bbdb9}, {0x01db5440, 0x003b6a88},
    {0xf0fed886, 0x003b183c}, {0x94bdc3f4, 0x003aabe3},
    {0xe76156da, 0x003a5ba3}, {0x953378db, 0x003a0c3e},
    {0x61320b1e, 0x0038f035}, {0xaef5908a, 0x0038d6ec},
    {0x221e6069, 0x003859cf}, {0x5dc9588a, 0x0037f741},
    {0xd3902626, 0x00377df0}, {0x136907fa, 0x00373622},
    {0x3b39b92f, 0x0036ef0c}, {0x47d55e6d, 0x0036915f},
    {0xf3f866fd, 0x0036072c}, {0x37be5ea8, 0x0035d9b7},
    {0x59cc81c7, 0x00359615}, {0x897a4592, 0x0035531c},  /* 200 */
    {0xbd3e98a4, 0x00353cee}, {0x81585e5e, 0x0034fad3},
    {0xd1103130, 0x00347884}, {0xac39bf56, 0x00340dd3},
    {0xfecc140c, 0x003351fd}, {0xb089b524, 0x00333d72},
    {0x44d6b261, 0x0033148d}, {0xf8412458, 0x0032d7ae},
    {0x0e79c0f1, 0x0032c385}, {0xd59048a2, 0x00328766},
    {0x8cb11833, 0x00325fa1}, {0x59327e22, 0x00324bd6},
    {0x784360f4, 0x0032246e}, {0xf1a33a08, 0x0031afa5},
    {0xff398e70, 0x00319c63}, {0x519a86a7, 0x003162f7},
    {0xc9d3fc3c, 0x0030271f}, {0xae89750b, 0x002ff104},
    {0xa236d133, 0x002fbb62}, {0x7d2070b4, 0x002f7499},
    {0xa8b6fce3, 0x002ed84a}, {0xf7a46dbd, 0x002e832d},
    {0x46857cab, 0x002e0e08}, {0xdfb55ee6, 0x002decfb},
    {0x6f3ff488, 0x002ddc87}, {0xd4c482c4, 0x002dbbc1},
    {0xe0de0556, 0x002d8af0}, {0x7d14b30a, 0x002d4a7b},
    {0x073bcf4e, 0x002d2a85}, {0xb13e8be4, 0x002d1a9a},
    {0xb4b9fd8b, 0x002ceb1e}, {0x3a79794c, 0x002c8d50},
    {0x708784ed, 0x002c404d}, {0x6315ec52, 0x002c3106},
    {0xd80f2664, 0x002c1297}, {0x44c55f6b, 0x002c0370},
    {0x4cd13086, 0x002be540}, {0xadaf0cce, 0x002bb845},
    {0xc639f16d, 0x002b5f62}, {0x734f2b88, 0x002b07e6},
    {0x9d8342b7, 0x002ace56}, {0x5dbd4dcf, 0x002a791d},
    {0x8113017c, 0x002a4eff}, {0xe156df32, 0x002a3319},
    {0x286526ea, 0x002a0986}, {0x51d91e39, 0x0029d295},
    {0x9e109f0a, 0x0029b752}, {0x491ea465, 0x00298137},
    {0x1eb9f9da, 0x0029665e}, {0x2e019a5e, 0x00290975},  /* 250 */
    {0xe2e5efb0, 0x0028ef35}, {0xaa4b8278, 0x0028c815},
    {0x867199da, 0x0028bb1b}, {0xf5d7b002, 0x0028a13f},
    {0xf173e755, 0x00287ab3}, {0xd67713bd, 0x00286dea},
    {0xcda6503e, 0x002847bf}, {0xea6b4777, 0x002808c1},
    {0x0f23ff61, 0x00278d0e}, {0x3c093c7f, 0x00276886},
    {0x15a73ca8, 0x00275051}, {0xa61dc1b9, 0x00274441},
    {0x66113cf0, 0x0026b5c1}, {0xad07b18e, 0x00269e65},
    {0x5f877560, 0x002692c2}, {0x7523cd11, 0x002658fa},
    {0x10cf0f9e, 0x00261487}, {0x3b22524f, 0x00260936},
    {0x5a1c1122, 0x0025d106}, {0x382b863f, 0x0025a48a},
    {0x90eccdbc, 0x00258371}, {0xe95d510c, 0x00256292},
    {0xa98d068c, 0x002541ed}, {0x87fed8f5, 0x0024e150},
    {0x20979e5d, 0x0024c18b}, {0x336de0c5, 0x0024ac7b},
    {0x478c60bb, 0x0024a1fc}, {0x1231c009, 0x00246380},
    {0xd506ed33, 0x0024300f}, {0xa494da81, 0x0023f314},
    {0xdd2fad3a, 0x0023cade}, {0xd2664a03, 0x00237b7e},
    {0x67dbaf1d, 0x00233729}, {0x8a371f20, 0x00231a30},
    {0x63e1e600, 0x002306fa}, {0x31575684, 0x0022fd67},
    {0x7805749c, 0x0022ea50}, {0xe8b3d720, 0x0022e0cc},
    {0x7857d161, 0x0022b188}, {0xfcc49cc0, 0x00227977},
    {0x7b5e5f4f, 0x00225db3}, {0x91322ed6, 0x0022421b},
    {0x35f52102, 0x0021f05b}, {0xe5c70d60, 0x0021e75d},
    {0x6c19be96, 0x0021a01d}, {0x6615c81a, 0x0021974a},
    {0x697cf36a, 0x00213767}, {0x7fad35f1, 0x00211d9f},
    {0x9dd36c18, 0x0020fb7d}, {0x3d661e0e, 0x0020e212},  /* 300 */
    {0xb66ae990, 0x0020d135}, {0xed4d7a8e, 0x0020c8cd},
    {0x3f43ddbf, 0x0020b80b}, {0x180f46a6, 0x002096b9},
    {0xe28de5da, 0x00207de7}, {0xc8cf1fb3, 0x002054de},
    {0x30b3aab5, 0x00204cb6}, {0xadc37beb, 0x00202428},
    {0x7834def4, 0x001fec0c}, {0xae98a1d0, 0x001fc46f},
    {0x430ff619, 0x001facda}, {0xdd8e15e5, 0x001f7e17},
    {0x3556a4ee, 0x001f765a}, {0x49d802f1, 0x001f66ea},
    {0x00faf9c0, 0x001f5f38}, {0xe6c0f1f9, 0x001f38f4},
    {0x46752578, 0x001f0b85}, {0x83f001f0, 0x001f03ff},
    {0xb0a3883c, 0x001ec853}, {0x573723eb, 0x001ec0ee},
    {0x8e6f6894, 0x001eaad3}, {0xa765fe53, 0x001e9c28},
    {0x758c2003, 0x001e94d8}, {0xa8f65e68, 0x001e707b},
    {0xa68f574e, 0x001e53a2}, {0xa56b438d, 0x001e1380},
    {0x513a3802, 0x001dbf9f}, {0xd58bc600, 0x001db1d1},
    {0x8f53de38, 0x001d9d35}, {0xdf6165c7, 0x001d81e6},
    {0x7fd40e30, 0x001d4bdf}, {0x7a1c958d, 0x001d452c},
    {0x9b902659, 0x001d37cf}, {0x5791e97b, 0x001d1d3a},
    {0xe6b47416, 0x001ce89f}, {0xf3235071, 0x001ce219},
    {0xdcf92139, 0x001cd516}, {0xbd1c2b8b, 0x001cbb33},
    {0xd2546688, 0x001ca7e7}, {0xc1b3dbd3, 0x001c94b5},
    {0xf9c241c1, 0x001c87f7}, {0x706c35a9, 0x001c6202},
    {0xa9437632, 0x001c5bb8}, {0x43b4111e, 0x001c1743},
    {0xd3e46b42, 0x001c04d0}, {0x0fbf4308, 0x001bfeb0},
    {0xce0b202d, 0x001bec5d}, {0x44620037, 0x001be034},
    {0xc66f6fc3, 0x001bce09}, {0x28d02b30, 0x001ba402},  /* 350 */
    {0xb1cf8919, 0x001b9225}, {0x2ff3f53f, 0x001b864a},
    {0x4150e49b, 0x001b8060}, {0xaaeaacf3, 0x001b6eb1},
    {0x8da3c8cc, 0x001b62f4}, {0xabe96092, 0x001b516b},
    {0xef1e0c87, 0x001b2e9c}, {0xbedc849b, 0x001b1d56},
    {0x7546aec0, 0x001b0c26}, {0x62024fa0, 0x001ae45f},
    {0x631b5f54, 0x001ad917}, {0x18cb608f, 0x001ac83d},
    {0xad8c063f, 0x001aa6c7}, {0xb1228e2a, 0x001a90a7},
    {0xc03ba059, 0x001a8027}, {0x289deb89, 0x001a7533},
    {0xce16b49f, 0x001a2ed7}, {0x0a279a73, 0x0019fefc},
    {0xcd873b5f, 0x0019e4b0}, {0xfd60e514, 0x0019cfcd},
    {0x32d66c85, 0x0019c569}, {0xab6fc7c2, 0x0019b5e1},
    {0xa62f2a73, 0x0019b0b8}, {0xfc98942c, 0x0019a149},
    {0x7ec25b85, 0x00196951}, {0x83360ba8, 0x00194b30},
    {0xf4bebdc1, 0x00194631}, {0x127268fd, 0x00191e84},
    {0xb543984f, 0x00190adb}, {0x0bd18200, 0x00190113},
    {0xb889ac94, 0x0018e3e6}, {0x420e1ec1, 0x0018c233},
    {0x72d92bd6, 0x0018aa58}, {0x9945ccf9, 0x0018a598},
    {0x60b57f60, 0x00189c1e}, {0xbc8690b9, 0x0018893f},
    {0xb3e1041c, 0x00187b2b}, {0xc9cdcfb8, 0x00186d27},
    {0xbf4f2c1c, 0x001863d8}, {0xe2ad7593, 0x00185f33},
    {0x75973e13, 0x001855ef}, {0x0153f134, 0x00184816},
    {0x2e6f0656, 0x001835b7}, {0x2d83eb39, 0x00182c92},
    {0x43c0365a, 0x00182802}, {0xd5898e73, 0x00181a5c},
    {0x961773aa, 0x001803c0}, {0x05ffd001, 0x0017ff40},
    {0x70433edb, 0x0017e8d6}, {0x6cf4bb5d, 0x0017d706},  /* 400 */
};

/* (0xFFFFFFFFFFFFFFFF%prime)+1 */
const uint16_t primetable_r[PTL_MAX] = {
    1,     1,     2,     5,     3,     1,     17,    6,     24,    16,
    12,    16,    41,    25,    15,    5,     16,    17,    10,    2,
    51,    36,    67,    61,    79,    55,    92,    66,    30,    2,
    65,    60,    13,    102,   16,    14,    57,    49,    47,    124,
    44,    26,    84,    61,    126,   69,    49,    104,   44,    64,    /* 50 */
    150,   225,   69,    1,     104,   57,    265,   175,   101,   240,
    109,   97,    208,   142,   251,   16,    2,     167,   219,   187,
    303,   297,   21,    277,   143,   169,   99,    63,    80,    409,
    26,    337,   296,   433,   215,   359,   215,   370,   261,   369,
    403,   286,   263,   31,    387,   302,   143,   141,   240,   60,    /* 100 */
    442,   438,   543,   443,   435,   339,   399,   51,    157,   359,
    374,   267,   94,    558,   1,     40,    380,   566,   51,    229,
    255,   122,   171,   681,   141,   312,   149,   511,   625,   94,
    583,   250,   601,   385,   361,   54,    766,   559,   783,   571,
    187,   813,   33,    391,   73,    435,   735,   408,   280,   301,   /* 150 */
    512,   514,   832,   579,   251,   487,   719,   718,   139,   421,
    417,   566,   632,   581,   536,   827,   961,   384,   223,   345,
    433,   809,   433,   49,    584,   241,   460,   505,   841,   164,
    487,   199,   428,   64,    898,   92,    434,   557,   1054,  630,
    765,   194,   690,   978,   1017,  675,   823,   856,   591,   534,   /* 200 */
    868,   970,   1232,  1038,  36,    292,   989,   232,   165,   830,
    1233,  242,   580,   200,   16,    1111,  1284,  579,   377,   1276,
    891,   707,   1403,  1006,  1240,  1244,  406,   634,   230,   748,
    207,   1100,  235,   250,   292,   677,   642,   1478,  1445,  1512,
    915,   1367,  1204,  430,   1274,  281,   1442,  1289,  1274,  922,   /* 250 */
    1104,  184,   1238,  870,   369,   1087,  1526,  1037,  1319,  767,
    1544,  1251,  208,   434,   992,   1411,  722,   1611,  1238,  1421,
    1548,  1236,  1036,  859,   1605,  217,   1329,  175,   55,    865,
    1578,  1115,  559,   416,   1536,  1084,  1076,  1056,  831,   576,
    1667,  730,   1514,  544,   1026,  474,   14,    501,   440,   1282,  /* 300 */
    1968,  1326,  659,   1534,  1382,  431,   879,   835,   1340,  1232,
    2035,  1403,  886,   1353,  64,    869,   1912,  528,   1284,  719,
    1164,  985,   1763,  1880,  1682,  1753,  1738,  1536,  1512,  133,
    1680,  973,   565,   887,   302,   1651,  1255,  1661,  1800,  1809,
    839,   691,   674,   410,   1018,  2008,  881,   1767,  417,   624,   /* 350 */
    1759,  1293,  299,   2129,  1300,  2002,  1171,  1173,  1216,  480,
    2060,  2335,  219,   322,   1087,  875,   359,   2437,  963,   676,
    725,   342,   11,    1156,  801,   1704,  31,    1427,  105,   1536,
    460,   617,   2026,  437,   96,    1481,  1844,  664,   1564,  749,
    545,   196,   158,   1889,  54,    2195,  198,   1365,  297,   2647,  /* 400 */
};

/* function: get a%prime, prime number count is PTL
 */
uint32_t bigint_div_table_high(uint32_t *a, uint32_t awordlen, uint16_t *r, double_uint32_t *s,
                               uint16_t *high_result, uint32_t PTL)
{
    uint32_t tmp_step, i, ret;
    uint32_t *p, *q;

    pke_set_operand_width(awordlen << 5);
    tmp_step = pke_get_operand_bytes();

    p = (uint32_t *)(PKE_B(1, tmp_step));
    q = (uint32_t *)(PKE_A(2, tmp_step));

    pke_load_operand((uint32_t *)(PKE_A(1, tmp_step)), a, awordlen);

    /* clear the high part(this action can not be deleted) */
    if (tmp_step > awordlen) {
        uint32_clear((uint32_t *)(PKE_A(1, tmp_step)) + awordlen, (tmp_step / 4) - awordlen);
    }

    p[1] = 0;
    p[3] = 0;

    for (i = 0; i < PTL; i++) {
        p[0] = primetable[i];
        p[2] = r[i];
        p[4] = s[i].low;
        p[5] = s[i].high;

        pke_set_microcode(MICROCODE_MODRES);    /* must be called every time */

        pke_clear_interrupt();

        pke_start();

        pke_wait_till_done();

        ret = pke_check_rt_code();

        if (PKE_SUCCESS != ret) {
            return ret;
        }

        high_result[i] = *q;
    }

    return PKE_SUCCESS;
}

/* function: (get high_result||a[0])%prime, prime number count is PTL
 */
uint32_t bigint_div_table_low(uint32_t *a, uint16_t *r, double_uint32_t *s, uint16_t *high_result, uint32_t PTL)
{
    volatile uint32_t MC_flag = MICROCODE_MODRES;
    volatile uint32_t start_flag = PKE_START_CALC;
    uint32_t tmp_step, i;
    uint32_t *p, *q, *k;

    pke_set_operand_width(2 << 5);
    tmp_step = pke_get_operand_bytes();

    p = (uint32_t *)(PKE_B(1, tmp_step));
    q = (uint32_t *)(PKE_A(2, tmp_step));
    k = (uint32_t *)(PKE_A(1, tmp_step));

    *k = *a;
    uint32_clear(k + 2, (tmp_step / 4) - 2);

    p[1] = 0;
    p[3] = 0;

    for (i = 0; i < PTL; i++) {
        *(k + 1) = high_result[i];

        p[0] = primetable[i];
        p[2] = r[i];
        p[4] = s[i].low;
        p[5] = s[i].high;

        PKE_MC_PTR = MC_flag;

        pke_clear_interrupt();

        PKE_CTRL |= start_flag;

        pke_wait_till_done();

        if (PKE_SUCCESS != PKE_RT_CODE) {
            return PKE_RT_CODE;
        }

        if (0 == (*q)) {
            return NOT_PRIME;
        }
    }

    return MAYBE_PRIME;
}

#elif(BIGINT_DIV_CHOICE == 2)
/* 0xFFFFFFFF/prime  */
const uint32_t primetable_s[PTL_MAX] = {
    0x55555555, 0x33333333, 0x24924924, 0x1745d174, 0x13b13b13, 0x0f0f0f0f, 0x0d79435e, 0x0b21642c, 0x08d3dcb0, 0x08421084,
    0x06eb3e45, 0x063e7063, 0x05f417d0, 0x0572620a, 0x04d4873e, 0x0456c797, 0x04325c53, 0x03d22635, 0x039b0ad1, 0x0381c0e0,
    0x033d91d2, 0x03159721, 0x02e05c0b, 0x02a3a0fd, 0x0288df0c, 0x027c4597, 0x02647c69, 0x02593f69, 0x0243f6f0, 0x02040810,
    0x01f44659, 0x01de5d6e, 0x01d77b65, 0x01b7d6c3, 0x01b20364, 0x01a16d3f, 0x01920fb4, 0x01886e5f, 0x017ad220, 0x016e1f76,
    0x016a13cd, 0x01571ed3, 0x01539094, 0x014cab88, 0x0149539e, 0x013698df, 0x0125e227, 0x0120b470, 0x011e2ef3, 0x01194538, /* 50 */
    0x0112358e, 0x010fef01, 0x0105197f, 0x00ff00ff, 0x00f92fb2, 0x00f3a0d5, 0x00f1d48b, 0x00ec9791, 0x00e93965, 0x00e79372,
    0x00dfac1f, 0x00d578e9, 0x00d2ba08, 0x00d16154, 0x00cebcf8, 0x00c5fe74, 0x00c27806, 0x00bcdd53, 0x00bbc840, 0x00b9a786,
    0x00b68d31, 0x00b2927c, 0x00afb321, 0x00aceb0f, 0x00ab1cbd, 0x00a87917, 0x00a513fd, 0x00a36e71, 0x00a03c16, 0x009c6916,
    0x009baade, 0x00980e41, 0x00975a75, 0x009548e4, 0x0093efd1, 0x0091f5bc, 0x008f67a1, 0x008e2917, 0x008d8be3, 0x008c5584,
    0x0088d180, 0x00869222, 0x0085797b, 0x008355ac, 0x00824a4e, 0x0080c121, 0x007dc9f3, 0x007d4ece, 0x0079237d, 0x0077cf53, /* 100 */
    0x0075a8ac, 0x007467ac, 0x00732d70, 0x0072c62a, 0x007194a1, 0x006fa549, 0x006e8419, 0x006d68b5, 0x006d0b80, 0x006bf790,
    0x006ae907, 0x006a3799, 0x0069dfbd, 0x0067dc4c, 0x00663d80, 0x0065ec17, 0x00654ac8, 0x00645c85, 0x00637299, 0x00632591,
    0x006160ff, 0x0060cdb5, 0x005ff401, 0x005ed79e, 0x005d7d42, 0x005c6f35, 0x005b2618, 0x005a2553, 0x0059686c, 0x0058ae97,
    0x0058345f, 0x005743d5, 0x005692c4, 0x00561e46, 0x005538ed, 0x0054c807, 0x005345ef, 0x00523a75, 0x00510237, 0x0050cf12,
    0x004fd319, 0x004fa170, 0x004f3ed6, 0x004f0de5, 0x004e1cae, 0x004cd47b, 0x004c78ae, 0x004c4b19, 0x004bf093, 0x004aba3c, /* 150 */
    0x004a6360, 0x004a383e, 0x0049e28f, 0x0048417b, 0x0047f043, 0x00474ff2, 0x00468b6f, 0x0045f13f, 0x0045a522, 0x0045342c,
    0x0044c4a2, 0x0043c5c2, 0x00437e49, 0x0043142d, 0x0042ab5c, 0x00422195, 0x0041bbb2, 0x0040f391, 0x0040b1e9, 0x00405064,
    0x00403024, 0x003f90c2, 0x003f7141, 0x003f1377, 0x003e7988, 0x003e5b19, 0x003dc4a5, 0x003da6e4, 0x003d4e4f, 0x003c4a6b,
    0x003c11d5, 0x003bf5b1, 0x003bbdb9, 0x003b6a88, 0x003b183c, 0x003aabe3, 0x003a5ba3, 0x003a0c3e, 0x0038f035, 0x0038d6ec,
    0x003859cf, 0x0037f741, 0x00377df0, 0x00373622, 0x0036ef0c, 0x0036915f, 0x0036072c, 0x0035d9b7, 0x00359615, 0x0035531c, /* 200 */
    0x00353cee, 0x0034fad3, 0x00347884, 0x00340dd3, 0x003351fd, 0x00333d72, 0x0033148d, 0x0032d7ae, 0x0032c385, 0x00328766,
    0x00325fa1, 0x00324bd6, 0x0032246e, 0x0031afa5, 0x00319c63, 0x003162f7, 0x0030271f, 0x002ff104, 0x002fbb62, 0x002f7499,
    0x002ed84a, 0x002e832d, 0x002e0e08, 0x002decfb, 0x002ddc87, 0x002dbbc1, 0x002d8af0, 0x002d4a7b, 0x002d2a85, 0x002d1a9a,
    0x002ceb1e, 0x002c8d50, 0x002c404d, 0x002c3106, 0x002c1297, 0x002c0370, 0x002be540, 0x002bb845, 0x002b5f62, 0x002b07e6,
    0x002ace56, 0x002a791d, 0x002a4eff, 0x002a3319, 0x002a0986, 0x0029d295, 0x0029b752, 0x00298137, 0x0029665e, 0x00290975, /* 250 */
    0x0028ef35, 0x0028c815, 0x0028bb1b, 0x0028a13f, 0x00287ab3, 0x00286dea, 0x002847bf, 0x002808c1, 0x00278d0e, 0x00276886,
    0x00275051, 0x00274441, 0x0026b5c1, 0x00269e65, 0x002692c2, 0x002658fa, 0x00261487, 0x00260936, 0x0025d106, 0x0025a48a,
    0x00258371, 0x00256292, 0x002541ed, 0x0024e150, 0x0024c18b, 0x0024ac7b, 0x0024a1fc, 0x00246380, 0x0024300f, 0x0023f314,
    0x0023cade, 0x00237b7e, 0x00233729, 0x00231a30, 0x002306fa, 0x0022fd67, 0x0022ea50, 0x0022e0cc, 0x0022b188, 0x00227977,
    0x00225db3, 0x0022421b, 0x0021f05b, 0x0021e75d, 0x0021a01d, 0x0021974a, 0x00213767, 0x00211d9f, 0x0020fb7d, 0x0020e212, /* 300 */
    0x0020d135, 0x0020c8cd, 0x0020b80b, 0x002096b9, 0x00207de7, 0x002054de, 0x00204cb6, 0x00202428, 0x001fec0c, 0x001fc46f,
    0x001facda, 0x001f7e17, 0x001f765a, 0x001f66ea, 0x001f5f38, 0x001f38f4, 0x001f0b85, 0x001f03ff, 0x001ec853, 0x001ec0ee,
    0x001eaad3, 0x001e9c28, 0x001e94d8, 0x001e707b, 0x001e53a2, 0x001e1380, 0x001dbf9f, 0x001db1d1, 0x001d9d35, 0x001d81e6,
    0x001d4bdf, 0x001d452c, 0x001d37cf, 0x001d1d3a, 0x001ce89f, 0x001ce219, 0x001cd516, 0x001cbb33, 0x001ca7e7, 0x001c94b5,
    0x001c87f7, 0x001c6202, 0x001c5bb8, 0x001c1743, 0x001c04d0, 0x001bfeb0, 0x001bec5d, 0x001be034, 0x001bce09, 0x001ba402, /* 350 */
    0x001b9225, 0x001b864a, 0x001b8060, 0x001b6eb1, 0x001b62f4, 0x001b516b, 0x001b2e9c, 0x001b1d56, 0x001b0c26, 0x001ae45f,
    0x001ad917, 0x001ac83d, 0x001aa6c7, 0x001a90a7, 0x001a8027, 0x001a7533, 0x001a2ed7, 0x0019fefc, 0x0019e4b0, 0x0019cfcd,
    0x0019c569, 0x0019b5e1, 0x0019b0b8, 0x0019a149, 0x00196951, 0x00194b30, 0x00194631, 0x00191e84, 0x00190adb, 0x00190113,
    0x0018e3e6, 0x0018c233, 0x0018aa58, 0x0018a598, 0x00189c1e, 0x0018893f, 0x00187b2b, 0x00186d27, 0x001863d8, 0x00185f33,
    0x001855ef, 0x00184816, 0x001835b7, 0x00182c92, 0x00182802, 0x00181a5c, 0x001803c0, 0x0017ff40, 0x0017e8d6, 0x0017d706, /* 400 */
};

/* (0xFFFFFFFF%prime)+1 */
const uint16_t primetable_r[PTL_MAX] = {
    1,     1,     4,     4,     9,     1,     6,     12,    16,    4,
    7,     37,    16,    42,    42,    51,    57,    33,    9,     32,
    50,    77,    45,    35,    68,    63,    29,    75,    16,    16,
    117,   34,    41,    129,   4,     93,    100,   7,     96,    126,
    15,    147,   108,   88,    46,    51,    7,     176,   161,   8,     /* 50 */
    110,   15,    123,   1,     34,    47,    219,   27,    35,    250,
    133,   149,   72,    76,    232,   4,     26,    127,   192,   58,
    73,    60,    235,   203,   317,   13,    167,   255,   218,   254,
    234,   145,   27,    260,   341,   324,   407,   405,   115,   52,
    384,   338,   279,   444,   190,   355,   117,   294,   215,   423,   /* 100 */
    452,   188,   528,   82,    287,   413,   535,   125,   128,   400,
    573,   63,    513,   172,   640,   571,   136,   191,   37,    155,
    417,   87,    341,   134,   582,   567,   664,   331,   708,   539,
    71,    549,   620,   490,   19,    733,   579,   447,   49,    506,
    211,   240,   686,   367,   446,   553,   386,   797,   115,   116,   /* 150 */
    672,   550,   647,   311,   403,   578,   561,   105,   518,   316,
    238,   50,    285,   67,    444,   53,    966,   383,   259,   500,
    108,   690,   183,   7,     440,   93,    39,    836,   29,    939,
    321,   843,   575,   8,     1044,  649,   1015,  658,   437,   788,
    155,   429,   976,   90,    276,   337,   1156,  265,   429,   660,   /* 200 */
    910,   625,   1020,  847,   1271,  882,   345,   1250,  73,    1082,
    715,   454,   614,   1245,  1317,  423,   1073,  932,   870,   675,
    922,   1363,  392,   1247,  621,   1191,  1264,  707,   41,    1006,
    1030,  336,   651,   574,   1255,  400,   448,   1017,  1170,  686,
    942,   565,   781,   1367,  246,   501,   970,   451,   190,   287,   /* 250 */
    1419,  1069,  845,   1549,  1527,  1358,  1307,  1499,  98,    390,
    141,   1083,  675,   1147,  634,   782,   113,   398,   610,   382,
    989,   1598,  1165,  944,   227,   359,   500,   128,   1507,  1172,
    1582,  1518,  755,   1008,  730,   361,   880,   1708,  888,   1877,
    919,   1085,  407,   1735,  823,   778,   813,   987,   1225,  478,   /* 300 */
    1423,  1853,  495,   189,   1785,  1590,  386,   1384,  964,   1407,
    542,   1801,  434,   602,   8,     1892,  581,   1089,  1469,  726,
    1189,  1400,  984,   1421,  1406,  1408,  699,   1841,  1239,  1938,
    1117,  1068,  1363,  770,   2043,  2155,  1962,  1685,  1879,  1735,
    2241,  1014,  1528,  617,   1936,  144,   1889,  628,   1827,  378,   /* 350 */
    1651,  446,   608,   1595,  1324,  1611,  2252,  1802,  1110,  933,
    945,   237,   1667,  1707,  1857,  393,   2015,  100,   2032,  2513,
    505,   1707,  1656,  2523,  1277,  1328,  2479,  188,   1853,  121,
    1898,  683,   1192,  1592,  1006,  1967,  1881,  2115,  2008,  2381,
    1237,  14,    491,   482,   718,   2268,  1600,  64,    1202,  1170,  /* 400 */
};

uint32_t bigint_div_table_high(uint32_t *a, uint32_t awordlen, uint16_t *r, double_uint32_t *s,
                               uint16_t *high_result, uint32_t PTL)
{
    int32_t j;
    uint32_t i;
    uint64_t carry, a1, a2, high1;

    for (i = 0; i < PTL; i++) {
        carry = 0;

        for (j = awordlen - 1; j >= 0; j--) {
            a1 = carry * r[i] + a[j];

            while (a1 > 0xFFFFFFFF) {
                a2 = a1 & 0xFFFFFFFF;
                high1 = a1 >> 32;
                a1 = high1 * r[i] + a2;
            }

            a2 = (a1 * s[i]) >> 32;
            carry = a1 - a2 * primetable[i];

            if (carry >= primetable[i]) {
                carry -= primetable[i];
            }
        }

        high_result[i] = (uint16_t)carry;
    }

    return PKE_SUCCESS;
}

uint32_t bigint_div_table_low(uint32_t *a, uint16_t *r, double_uint32_t *s, uint16_t *high_result, uint32_t PTL)
{
    uint32_t i;
    uint64_t carry, a1, a2, high1;

    for (i = 0; i < PTL; i++) {
        carry = high_result[i];
        {
            a1 = carry * r[i] + a[0];

            while (a1 > 0xFFFFFFFF) {
                a2 = a1 & 0xFFFFFFFF;
                high1 = a1 >> 32;
                a1 = high1 * r[i] + a2;
            }

            a2 = (a1 * s[i]) >> 32;
            carry = a1 - a2 * primetable[i];

            if (carry >= primetable[i]) {
                carry -= primetable[i];
            }
        }

        if (0 == (uint32_t)carry) {
            return NOT_PRIME;
        }
    }

    return MAYBE_PRIME;
}
#endif

#if (PRIMALITY_TEST_CHOICE == 1)
/* function: for given odd number, according to Fermat Method, test whether it is prime or not
 * parameters:
 *     p ---------------- pointer to uint32_t big odd number p
 *     pbitlen ---------- bit length of p
 *     round ------------ round of test
 * return:
 *     0 ---------------- p is prime number with high probability
 *     0xFFFFFFFF ------- p is composite number
 *     other ------------ error
 * caution:
 *     1. make sure p is odd, and pwordlen>1
 *     2. make sure round>0
 */
uint32_t primality_test_Fermat(uint32_t *p, uint32_t pbitlen, uint32_t round)
{
    uint32_t pwordlen = GET_WORD_LEN(pbitlen);
    uint32_t tmp_step, i, tag;
    uint32_t ret;

    pke_set_operand_width(pbitlen);
    tmp_step = pke_get_operand_bytes();

    for (i = 0; i < round; i++) {
        /* A2, exponent, make it to be (p-1) */
        uint32_copy((uint32_t *)(PKE_A(2, tmp_step)), p, pwordlen);
        *((uint32_t *)(PKE_A(2, tmp_step))) -= 1;

        /* B1, base, make it to be in [2, p-2] */
        uint32_clear((uint32_t *)(PKE_B(1, tmp_step)), pwordlen);

GET_RAND_BASE:

        ret = get_rand((uint8_t *)&tag, 4);

        if (TRNG_SUCCESS != ret) {
            return ret;
        }

        if (tag < 2) {
            goto GET_RAND_BASE;
        }

        *((uint32_t *)(PKE_B(1, tmp_step))) = tag;
        uint32_clear((uint32_t *)(PKE_B(1, tmp_step)) + pwordlen - 1, (tmp_step / 4) - pwordlen + 1);

        /* get pre-calculated mont paras */
        ret = pke_pre_calc_mont(p, pbitlen, NULL);

        if (PKE_SUCCESS != ret) {
            return ret;
        }

        /* A1, base^d mod p */
        ret = pke_modexp((uint32_t *)(PKE_A(0, tmp_step)), (uint32_t *)(PKE_A(2, tmp_step)), (uint32_t *)(PKE_B(1, tmp_step)),
                         (uint32_t *)(PKE_A(1, tmp_step)), pwordlen, pwordlen);

        if (PKE_SUCCESS != ret) {
            return ret;
        }

        /* if the result is 1 mod p, then p is probablly prime */
        if (bigint_check_1((uint32_t *)(PKE_A(1, tmp_step)), pwordlen)) {
            continue;
        }
        else {
            return NOT_PRIME;
        }
    }

    return MAYBE_PRIME;
}

#elif (PRIMALITY_TEST_CHOICE == 2)
/* function: for given odd number, according to Miller_Rabin Method, test whether it is prime or not
 * parameters:
 *     p ---------------- pointer to uint32_t big odd number p
 *     pbitlen ---------- bit length of p
 *     round ------------ round of test
 * return:
 *     0 ---------------- p is prime number with high probability
 *     0xFFFFFFFF ------- p is composite number
 *     other ------------ error
 * caution:
 *     1. make sure p is odd, and pwordlen>1
 *     2. make sure round>0
 */
uint32_t primality_test_Miller_Rabin(uint32_t *p, uint32_t pbitlen, uint32_t round)
{
    uint32_t pwordlen = GET_WORD_LEN(pbitlen);
    uint32_t tmp_step, i, j, tag, even_num;
    int32_t ret;

    pke_set_operand_width(pbitlen);
    tmp_step = pke_get_operand_bytes();

    for (i = 0; i < round; i++) {
        /* A2, exponent, make it to be (p-1)=d*(2^t), where d is odd */
        uint32_copy((uint32_t *)(PKE_A(2, tmp_step)), p, pwordlen);
        *((uint32_t *)(PKE_A(2, tmp_step))) -= 1;

        /* make exponent to be d */
        even_num = get_multiple2_number((uint32_t *)(PKE_A(2, tmp_step)));
        big_div2n((uint32_t *)(PKE_A(2, tmp_step)), pwordlen, even_num);

        /* B1, base, make it to be in [2, p-2] */
        uint32_clear((uint32_t *)(PKE_B(1, tmp_step)), pwordlen);

GET_RAND_BASE:

        ret = get_rand((uint8_t *)&tag, 4);

        if (TRNG_SUCCESS != ret) {
            return ret;
        }

        if (tag < 2) {
            goto GET_RAND_BASE;
        }

        *((uint32_t *)(PKE_B(1, tmp_step))) = tag;
        uint32_clear((uint32_t *)(PKE_B(1, tmp_step)) + pwordlen - 1, (tmp_step / 4) - pwordlen + 1);

        /* get pre-calculated mont paras */
        ret = pke_pre_calc_mont(p, pbitlen, NULL);

        if (PKE_SUCCESS != ret) {
            return ret;
        }

        /* A1, base^d mod p */
        ret = pke_modexp((uint32_t *)(PKE_A(0, tmp_step)), (uint32_t *)(PKE_A(2, tmp_step)), (uint32_t *)(PKE_B(1, tmp_step)),
                         (uint32_t *)(PKE_A(1, tmp_step)), pwordlen, pwordlen);

        if (PKE_SUCCESS != ret) {
            return ret;
        }

        /* if the result is 1 or -1 mod p, then p is probablly prime */
        if (bigint_check_1((uint32_t *)(PKE_A(1, tmp_step)), pwordlen)
                || bigint_check_p_1((uint32_t *)(PKE_A(1, tmp_step)), p, pwordlen)) {
            continue;
        }

        tag = 0;

        for (j = 1; j < even_num; j++) {

            ret = pke_modmul_internal((uint32_t *)(PKE_A(0, tmp_step)), (uint32_t *)(PKE_A(1, tmp_step)), (uint32_t *)(PKE_A(1, tmp_step)),
                                      (uint32_t *)(PKE_A(1, tmp_step)), pwordlen);

            if (PKE_SUCCESS != ret) {
                return ret;
            }

            /* if the result is -1 mod p, then p is probably prime */
            if (bigint_check_p_1((uint32_t *)(PKE_A(1, tmp_step)), p, pwordlen)) {
                tag = 1;
                break;
            }
        }

        if (1 == tag) {
            continue;
        }
        else {
            return NOT_PRIME;
        }
    }

    return MAYBE_PRIME;
}
#endif

/* function: get prime number of pbitlen
 * parameters:
 *     p -------------------------- pointer to uint32_t big prime number
 *     pbitlen -------------------- bit length of p
 * return:
 *     0 -------------------------- success, p is prime number with high probability
 *     other ---------------------- error
 * caution:
 *     1. pbitlen must be bigger than 32, but less than 2048
 */
uint32_t get_prime(uint32_t p[], uint32_t pbitlen)
{
    uint32_t ret;
    uint32_t bitlen, pwordlen = (pbitlen + 0x1F) >> 5;
    uint16_t high_result[PTL_MAX];
    uint32_t PTL;

    if (pbitlen <= 512) {
        PTL = PTL_512;
    }
    else if (pbitlen >= 1024) {
        PTL = PTL_1024;
    }
    else {
        PTL = ((PTL_1024 - PTL_512) / (1024 - 512)) * (pbitlen - 512) + PTL_512;
    }

    if (PTL > PTL_MAX) {
        PTL = PTL_MAX;
    }

#ifdef PRIME_TEST
    uint32_set(p, 0x22222222, pwordlen);
#else
    ret = get_rand((uint8_t *)p, pwordlen << 2);

    if (TRNG_SUCCESS != ret) {
        return ret;
    }

#endif

    /* make high two bit all 1 */
    bitlen = pbitlen & 0x1F;

    switch (bitlen) {
        case 0:
            p[pwordlen - 1] |= 0xC0000000;
            break;

        case 1:
            p[pwordlen - 1] = 1;
            p[pwordlen - 2] |= 0x80000000;
            break;

        default:
            p[pwordlen - 1] &= ((1 << bitlen) - 1);
            p[pwordlen - 1] |= (0x3 << (bitlen - 2));
            break;
    }

    /* make p odd */
    p[0] |= 0x01;

    ret = bigint_div_table_high(p + 1, pwordlen - 1, (uint16_t *)primetable_r, (double_uint32_t *)primetable_s, high_result, PTL);

    if (PKE_SUCCESS != ret) {
        return ret;
    }

ADD_2:
    p[0] += 2;
#ifdef PRIME_TEST
    xxxx++;
#endif
    ret = bigint_div_table_low(p, (uint16_t *)primetable_r, (double_uint32_t *)primetable_s, high_result, PTL);

    if (NOT_PRIME == ret) {
#ifdef PRIME_TEST
        xxxx2++;
#endif
        goto ADD_2;
    }
    else if (MAYBE_PRIME != ret) {
        return ret;
    }

#if (PRIMALITY_TEST_CHOICE == 1)
    ret = primality_test_Fermat(p, pbitlen, FERMAT_ROUND);
#elif (PRIMALITY_TEST_CHOICE == 2)
    ret = primality_test_Miller_Rabin(p, pbitlen, MILLER_RABIN_ROUND);
#endif

    if (NOT_PRIME == ret) {
        goto ADD_2;
    }

#ifdef PRIME_TEST
    else if (MAYBE_PRIME == ret) {
        count++;
        /* if(count % 20 == 0) */
        printf("\r\n %d  %d  %d  %08x", count, xxxx, xxxx2, p[0]);
        goto ADD_2;
    }

#endif

    return ret;
}

