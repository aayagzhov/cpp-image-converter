#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <cassert>
#include <iostream>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char signature[2];
    uint32_t global_size;
    uint32_t buf;
    uint32_t step;
}
PACKED_STRUCT_END

static_assert(sizeof(BitmapFileHeader) == 14);

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t info_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_pix;
    uint32_t compression_type;
    uint32_t data_bytes; // stride * heigh;
    int32_t hor_res;
    int32_t ver_res;
    int32_t use_colors;
    int32_t sig_colors;
}
PACKED_STRUCT_END

static_assert(sizeof(BitmapInfoHeader) == 40);

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    
    const int stride = GetBMPStride(image.GetWidth());
    const uint32_t data_bytes = stride * image.GetHeight();
    
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    /// Запись File Header
    file_header.signature[0] = 'B';
    file_header.signature[1] = 'M';
    file_header.global_size = (data_bytes + sizeof(file_header) + sizeof(info_header));
    file_header.buf = 0u;
    file_header.step = 54u;

    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));

    /// Запись File Header
    info_header.info_size = 40u;
    info_header.planes = 1u;
    info_header.bit_pix = 24u;
    info_header.compression_type = 0u;
    info_header.hor_res = 11811;
    info_header.ver_res = 11811;
    info_header.use_colors = 0;
    info_header.sig_colors = 0x1000000;
    info_header.width = image.GetWidth();
    info_header.height = image.GetHeight();
    info_header.data_bytes = data_bytes;

    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    /// Запись изображения
    vector<char> buffer(stride);
    for (int i = 0; i < image.GetHeight(); ++i) {
        const Color* line = image.GetLine(image.GetHeight() - i - 1);
        for (int j = 0; j < image.GetWidth(); ++j) {
            buffer[j * 3 + 0] = static_cast<char>(line[j].b);
            buffer[j * 3 + 1] = static_cast<char>(line[j].g);
            buffer[j * 3 + 2] = static_cast<char>(line[j].r);
        }
        out.write(buffer.data(), stride);
    }

    return true;
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);

    /// Чтение File Header
    BitmapFileHeader file_header;
    
    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    
    assert(file_header.signature[0] == 'B' && file_header.signature[1] == 'M');
    assert(file_header.buf == 0u);
    assert(file_header.step == 54u);

    /// Чтение Info Header
    BitmapInfoHeader info_header;

    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    assert(info_header.info_size == 40u);
    assert(info_header.planes == 1u);
    assert(info_header.bit_pix == 24u);
    assert(info_header.compression_type == 0u);
    assert(info_header.hor_res == 11811);
    assert(info_header.ver_res == 11811);
    assert(info_header.use_colors == 0);
    assert(info_header.sig_colors == 0x1000000);

    Image result(info_header.width, info_header.height, Color::Black());
    
    const int stride = GetBMPStride(info_header.width);
    vector<char> buffer(stride);
    for (int i{0}; i < info_header.height; ++i) {
        ifs.read(buffer.data(), stride);
        auto *line = result.GetLine(info_header.height - 1 - i);
        for (int j{0}; j < info_header.width; ++j) {
            line[j].b = static_cast<std::byte>(buffer[j * 3 + 0]); 
            line[j].g = static_cast<std::byte>(buffer[j * 3 + 1]);
            line[j].r = static_cast<std::byte>(buffer[j * 3 + 2]);
        }
    }
    return result;
}

}  // namespace img_lib