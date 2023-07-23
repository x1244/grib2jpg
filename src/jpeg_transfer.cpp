#include "jpeg_transfer.h"
#include <csetjmp>
#include <cstdio>
#include <format>
#include <stdexcept>
#include <icecream.hpp>
#include <jpeglib.h>
namespace {
#if 0
struct my_error_mgr{
struct jpeg_error_mgr pub;
jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr* my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}
#endif
}//namespace
void JpegTransfer::loadSource(const std::string& filename)
{
#if 0
    struct my_error_mgr my_err;
    cinfo.err = jpeg_std_error(&my_err.pub);
    my_err.pub.error_exit = my_error_exit;
    if(setjmp(my_err.setjmp_buffer)){
        cout <<"Error occured" <<endl;
        jpeg_destroy_decompress(&cinfo);
        return 0;
    }
#endif
    struct jpeg_decompress_struct src;
    struct jpeg_error_mgr err;
    src.err = jpeg_std_error(&err);
    jpeg_create_decompress(&src);
    std::FILE *f = std::fopen(filename.c_str(), "rb");
    if(!f){
        std::string msg = std::format("读取源文件 {} 失败", filename);
        throw std::runtime_error(msg);
    }
    jpeg_stdio_src(&src, f);
    int rr = jpeg_read_header(&src, true);
    if(rr != JPEG_HEADER_OK){
        jpeg_destroy_decompress(&src);
        std::fclose(f);
        std::string msg = std::format("获取源文件 {} JPEG头失败", filename);
        throw std::runtime_error(msg);
    }
    src_.w = src.image_width;
    src_.h = src.image_height;
    src_.c = src.num_components;
    bool stat = jpeg_start_decompress(&src);
    if(!stat){
        jpeg_destroy_decompress(&src);
        std::fclose(f);
        std::string msg = std::format("解压源文件 {} 失败", filename);
        throw std::runtime_error(msg);
    }
    unsigned long datasize = src_.w*src_.h*src_.c;
    src_.buff.resize(datasize);
    std::uint8_t* data = src_.buff.data();
    std::uint8_t* rowptr;
    while(src.output_scanline < src_.h){
        rowptr = data + src.output_scanline*src_.w*src_.c;
        jpeg_read_scanlines(&src, &rowptr, true);
    }
    jpeg_finish_decompress(&src);
}
void JpegTransfer::setSource(const std::uint8_t* p, int size, int w, int h, int c)
{
    src_.w = w;
    src_.h = h;
    src_.c = c;
    src_.buff.assign(p, p + size);
}
void JpegTransfer::transferUpper()
{
    int w = 2;
    while(w < src_.w || w > 8192){
        w = w<<1;
    }
    transfer(w, w>>1);
}
void JpegTransfer::transfer(int w, int h)
{
    const int c = 3;
    dest_.w = w;
    dest_.h = h;
    dest_.c = c;
    const int w0 = src_.w;
    const int h0 = src_.h;
    const int w2 = w, h2 = h, bitdepth = c*8;
    int sw = w0 - 1, sh = h0 - 1, dw = w2 - 1, dh = h2 - 1;
    int B, N, x, y;
    int pixelsize = c;
    unsigned char* lineprev, *linenext;
    dest_.buff.resize(w2*h2*pixelsize);
    std::uint8_t* data = src_.buff.data();
    unsigned char* dest = dest_.buff.data();
    unsigned char* tmp;
    unsigned char* pa, *pb, *pc, *pd;
    for(int i = 0; i <= dh; ++i){
        tmp = dest + i*w2*pixelsize;
        y = i*sh/dh;
        N = dh - i*sh%dh;
        lineprev = data + (y++)*w0*pixelsize;
        linenext = (N == dh) ? lineprev : data + y*w0*pixelsize;
        for(int j = 0; j <= dw; ++j){
            x = j*sw/dw*pixelsize;
            B = dw - j*sw%dw;
            pa = lineprev + x;
            pb = pa + pixelsize;
            pc = linenext + x;
            pd = pc + pixelsize;
            if(B == dw){
                pb = pa;
                pd = pc;
            }
            for(int k = 0; k < pixelsize; ++k){
                *tmp++ = (unsigned char)(int)(
                    (B*N*(*pa++ - *pb - *pc + *pd) + dw*N**pb++
                    + dh*B**pc++ + (dw*dh - dh*B - dw*N)**pd++
                    + dw*dh/2)/(dw*dh));
            }
        }
    }
}
void JpegTransfer::save(const std::string& filename, int q) const
{
    struct jpeg_compress_struct jcs;
    struct jpeg_error_mgr jem;
    jcs.err = jpeg_std_error(&jem);
    jpeg_create_compress(&jcs);
    std::FILE* outfile = std::fopen(filename.c_str(), "wb");
    if(!outfile){
        std::string msg = std::format("JPEG目标文件 {} 无法打开写入", filename);
        throw std::runtime_error(msg);
    }
    jpeg_stdio_dest(&jcs, outfile);
    const int w2 = dest_.w;
    const int h2 = dest_.h;
    const int c  = dest_.c;
    jcs.image_width = w2;
    jcs.image_height = h2;
    jcs.input_components = c;
    jcs.in_color_space = JCS_RGB;
    jpeg_set_defaults(&jcs);
    jpeg_set_quality(&jcs, q, true);
    jpeg_start_compress(&jcs, true);
    int row_stride = w2*c;
    JSAMPROW row_pointer[1];
    std::uint8_t* dest = const_cast<std::uint8_t*>(dest_.buff.data());
    while(jcs.next_scanline < jcs.image_height){
        row_pointer[0] = &dest[jcs.next_scanline*row_stride];
        (void)jpeg_write_scanlines(&jcs, row_pointer, 1);
    }
    jpeg_finish_compress(&jcs);
    jpeg_destroy_compress(&jcs);
}
void JpegTransfer::getSourePixelNum(int& w, int& h)
{
    w = src_.w;
    h = src_.h;
}
void JpegTransfer::getDestPixelNum(int& w, int& h)
{
    w = dest_.w;
    h = dest_.h;
}
