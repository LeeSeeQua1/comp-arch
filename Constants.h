//
// Created by Alex on 11/29/2023.
//

#ifndef UNTITLED1_CONSTANTS_H
#define UNTITLED1_CONSTANTS_H


class Constants {
public:

//    variant 1

    int ADDR_LEN = 19;
    static const int CACHE_WAY = 4;
    static const int CACHE_TAG_LEN = 10;
    static const int CACHE_IDX_LEN = 4;
    static const int CACHE_OFFSET_LEN = 5;
    int CACHE_SIZE = 2048;
    int CACHE_LINE_COUNT = 64;
    static const int CACHE_SETS_COUNT = 16;
    static const int CACHE_LINE_SIZE = 32;
    static const int ADDR1_BUS_LEN = 19;
    static const int ADDR2_BUS_LEN = 19;
    static const int DATA1_BUS_LEN = 16;
    static const int DATA2_BUS_LEN = 16;
    static const int CTR1_BUS_LEN = 3;
    static const int CTR2_BUS_LEN = 2;

//    variant 3

//    static const int ADDR_LEN = 20;
//    static const int CACHE_WAY = 4;
//    static const int CACHE_TAG_LEN = 9;
//    static const int CACHE_OFFSET_LEN = 7;
//    static const int CACHE_IDX_LEN = 4;
//    static const int CACHE_SETS_COUNT = 16;
//    static const int CACHE_LINE_SIZE = 128;
//    static const int CACHE_LINE_COUNT = 64;
//    static const int ADDR_BUS_LEN = 20;


//    variant 2

//    static const int ADDR_LEN = 20;
//    static const int CACHE_WAY = 4;
//    static const int CACHE_TAG_LEN = 10;
//    static const int CACHE_OFFSET_LEN = 6;
//    static const int CACHE_IDX_LEN = 4;
//    static const int CACHE_SETS_COUNT = 16;
//    static const int CACHE_LINE_SIZE = 64;
//    static const int CACHE_LINE_COUNT = 64;
//    static const int ADDR_BUS_LEN = 20;

//    variant 4

//    static const int ADDR_LEN = 20;
//    static const int CACHE_WAY = 2;
//    static const int CACHE_TAG_LEN = 11;
//    static const int CACHE_OFFSET_LEN = 5;
//    static const int CACHE_IDX_LEN = 4;
//    static const int CACHE_SETS_COUNT = 16;
//    static const int CACHE_LINE_SIZE = 32;
//    static const int CACHE_LINE_COUNT = 32;
//    static const int ADDR_BUS_LEN = 20;
};


#endif //UNTITLED1_CONSTANTS_H
