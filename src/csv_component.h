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
 * \param flip - 纬度上下翻转
 * \param shift - 经度左右互换
 * \param offset - 数值偏移量
 */
void init(int w, int h, bool flip, bool shift, double offset = 0.0);
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
/**\brief 计算分层值
 *
 * 由最小最大值计算分层值
 * \param minz - 最小值
 * \param maxz - 最大值
 * \return double - 分层值
 */
double stepValue(double minz, double maxz) const;
/**\brief 规划到单字节整数值
 *
 * \param v - 真值
 * \param minz - 下限
 * \param step - 分层值
 * \return std::uint_8 - 规整值
 */
std::uint8_t normal(double v, double minz, double step) const;
/**\brief 获取一个经度条带数据
 *
 * \param is - 输入流
 * \param p - 缓冲区起始地址
 * \param cout - 经度点数
 */
void fetchSegment(std::istream& is, std::uint8_t* p, int count) const;
/**\brief 正常获取
 *
 * \param is - 输入流
 * \param p - 缓冲区
 */
void doFetch(std::istream& is, std::uint8_t* p);
/**\brief 翻转纬度
 *
 * \param is - 输入流
 * \param p - 缓冲区
 */
void doFetchFlip(std::istream& is, std::uint8_t* p);
/**\brief 生成片段
 * 
 * \param is - 输入流
 * \param p - 缓冲区
 */
void doSegment(std::istream& is, std::uint8_t* p);
private:
bool flip_;                            /**< 纬度翻转 */
bool shift_;                           /**< 经度互换 */
int w_;                                /**< 水平分辨率 */
int h_;                                /**< 垂直分辨率 */
int p1_;                               /**< 左边点数 */
int p2_;                               /**< 右边点数 */
double minz_;                          /**< 数值下限 */
double step_;                          /**< 分层值 */
double offset_;                        /**< 偏移量 */
std::vector<std::uint8_t> buff_;       /**< 缓冲区 */
};
