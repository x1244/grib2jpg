/**\file 
 *
 * \brief grib2文件转csv格式的数值分量
 * \author x1244 <x11244@126.com>
 */
#pragma once
#include <istream>
#include <vector>
/**\brief 获取grib2的csv格式数据分量
 */
class CsvComponent
{
public:
/**\brief 初始化
 *
 * \param w - 水平分辨率
 * \param h - 垂直分辨率
 */
void init(int w, int h);
/**\brief 提取分量
 *
 * 提取中行无效抛出runtime_error异常；
 * 抛出invalid_argument异常，如果数值转换失败；
 * 抛出out_of_range异常，如果值无效。
 *
 * \param is - 输入流
 * \param c - 分量索引
 * \param minz - 分量最小值
 * \param maxz - 分量最大值
 */
void fetchCompent(std::istream& is, int c, double minz, double maxz);
/**\brief 缓冲区起始地址
 *
 * \param std::uint8_t* - 缓冲区
 */
const std::uint8_t* buff() const;
/**\brief 缓冲区大小
 *
 * \return int - 大小
 */
int size() const;
protected:
double stepValue(double minz, double maxz) const;
std::uint8_t normal(double v, double minz, double step) const;
void fetchSegment(std::istream& is, std::uint8_t* p, int count
    , double minz, double step) const;
private:
int w_;                                /**< 水平分辨率 */
int h_;                                /**< 垂直分辨率 */
std::vector<std::uint8_t> buff_;       /**< 缓冲区 */
};
