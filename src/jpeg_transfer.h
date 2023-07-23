/**\file JPEG文件变换器
 *
 * \brief 生成和变换JPEG文件
 * \author x1244 <x11244@126.com>
 */
#pragma once
#include <string>
#include <vector>
/**\brief JPEG图像生成与分辨率变换
 *
 * 处理过程中发生的任何问题都会抛出runtime_error。
 *
 * 主要参考网上资料
 * https://blog.csdn.net/qq160816/article/details/52688987
 * 以及
 * https://blog.csdn.net/wuyiyu_/article/details/128676240#:~:text=2.%20struct%20jpeg_error_mgr%20%E7%BB%93%E6%9E%84%E4%BD%93%3A%20%E8%AF%A5%E7%BB%93%E6%9E%84%E4%BD%93%E5%AE%9A%E4%B9%89%E7%9A%84%E5%8F%98%E9%87%8F%EF%BC%8C%E7%94%A8%E4%BA%8E%E5%A4%84%E7%90%86%20libjpeg%20%E5%BA%93%E8%A7%A3%E7%A0%81%20jpeg,libjpeg%20%E5%BA%93%E8%A7%A3%E7%A0%81%20jpeg%20%E6%95%B0%E6%8D%AE%E6%97%B6%EF%BC%8C%E5%8F%AF%E8%83%BD%E5%87%BA%E7%8E%B0%E5%86%85%E5%AD%98%E4%B8%8D%E8%B6%B3%E5%AF%BC%E8%87%B4%E9%94%99%E8%AF%AF%EF%BC%8C%E5%8F%AF%E4%BB%A5%20%E8%B0%83%E7%94%A8%E9%BB%98%E8%AE%A4%E9%94%99%E8%AF%AF%E5%A4%84%E7%90%86%E5%87%BD%E6%95%B0%E7%BB%A7%E8%80%8C%E4%BC%9A%E8%B0%83%E7%94%A8%20exit%20%E5%87%BD%E6%95%B0%E7%BB%93%E6%9D%9F%E6%95%B4%E4%B8%AA%E8%BF%9B%E7%A8%8B%20%E3%80%82
 */
class JpegTransfer
{
public:
/**\brief 由文件加载源
 *
 * \param filename - 文件名
 */
void loadSource(const std::string& filename);
/**\brief 由缓冲区加载源
 *
 * 目前只支持3通道RBG数据
 *
 * \param p - 缓冲区起始地址
 * \param size - 缓冲区大小
 * \param w - 宽
 * \param h - 高
 * \param c - 通道数量，1-gray 3-rgb
 */
void setSource(const std::uint8_t* p, int size, int w, int h, int c);
/**\brief 以向上规整的方式向上变换到2^*像素
 */
void transferUpper();
/**\brief JPEG变换
 *
 * \param w - 宽
 * \param h - 高
 */
void transfer(int w, int h);
/**\brief 保存文件
 *
 * \param filename - 文件名
 * \param q - 图像质量
 */
void save(const std::string& filename, int q) const;
/**\brief 获取源图像分辨率
 *
 * \param w - 宽
 * \param h - 高
 */
void getSourePixelNum(int& w, int& h);
/**\brief 获取目标图像分辨率
 *
 * \param w - 宽
 * \param h - 高
 */
void getDestPixelNum(int& w, int& h);
protected:
using Buff = std::vector<std::uint8_t>;
/**\brief JPEG参数
 */
struct Jpeg{
Buff buff;                             /**< 源数据缓冲区 */
int w;                                 /**< 图像宽 */
int h;                                 /**< 图像高 */
int c;                                 /**< 通道数量 */
};
private:
Jpeg src_;                             /**< 源 */
Jpeg dest_;                            /**< 目标 */
};
