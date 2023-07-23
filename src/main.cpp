#include <iostream>
#include <fstream>
#include <chrono>
#include <regex>
#include <thread>
#include <vector>
#include <icecream.hpp>
#include "csv_component.h"
#include "jpeg_transfer.h"
using namespace std;
using namespace std::chrono;
const int w = 1440;      //宽
const int h = 721;       //高
const int c = 3;         //分量
const int p1 = w/2 + 1;  //0 -> 180
const int p2 = w/2 - 1;  //-179.75 -> -0.25
const double minu = -23.0378;
const double maxu = 31.0522;
const double minv = -24.9615;
const double maxv = 23.7585;
const int valuecolumn = 6;
std::regex rgx{R"(,(-?\d*\.?\d*)$)"}; //匹配最后的数值
double stepValue(double minz, double maxz);
std::uint8_t normal(double v, double minz, double maxz);
/**\breif 提取经度条带
 *
 * \param is - 输入流
 * \param p - 目标缓冲区
 * \param cout - 提取点数
 * \param minz - 最小值
 * \param step - 分层值
 */
void fetchSegment(std::istream& is, std::uint8_t* p, int count, double minz, double step);
void fetchCompent(std::istream& is, std::uint8_t* p, double minz, double maxz);
void fetchCompentFlip(std::istream& is, std::uint8_t* p, double minz, double maxz);
int main(int argc, char** argv)
{
    std::vector<std::uint8_t> buff(w*h*c);
    std::ifstream is("z.csv");
    if(!is.is_open()){
        cout <<"无法打开z.dat" <<endl;
        return 0;
    }
#if 0
    std::uint8_t* p = buff.data();
    //U分量
    //fetchCompent(is, p, minu, maxu);
    fetchCompentFlip(is, p, minu, maxu);
    //V分量
    //fetchCompent(is, p + 1, minv, maxv);
    fetchCompentFlip(is, p + 1, minv, maxv);
#endif
    CsvComponent cc;
    cc.init(w, h);
    try{
        cc.fetchCompent(is, 0, minu, maxu);
        cc.fetchCompent(is, 1, minv, maxu);
    }
    catch(std::exception& ex){
        cout <<ex.what() <<endl;
        return 0;
    }
    JpegTransfer jpeg;
    jpeg.setSource(cc.buff(), cc.size(), w, h, c);
    jpeg.transferUpper();
    jpeg.save("uv.jpg", 90);
    cout <<"图像已保存至uv.jpg" <<endl;

    return 0;
}
void fetchSegment(std::istream& is, std::uint8_t* p, int count, double minz, double step)
{
    //正则表达式要慢很多，可查觉的慢
#ifdef REGEX
    std::smatch match;
#endif
    string linestr;
    while(std::getline(is, linestr)){
#ifdef REGEX
        bool m = std::regex_search(linestr, match, rgx);
        if(!m){
            IC(linestr);
            continue;
        }
        double v = std::stod(match[1]);
#else
        int pos = linestr.rfind(",");
        if(pos == std::string::npos){
            IC(linestr);
            continue;
        }
        std::string vstr(linestr.substr(pos + 1));
        double v = std::stod(vstr);
#endif
        *p = normal(v, minz, step);
        p += c;
        if(--count == 0) break;
    }
}
double stepValue(double minz, double maxz)
{
    return (maxz - minz)/255.;
}
std::uint8_t normal(double v, double minz, double step)
{
    return static_cast<std::uint8_t>((v - minz)/step);
}
void fetchCompent(std::istream& is, std::uint8_t* p, double minz, double maxz)
{
    //U分量
    double step = stepValue(minz, maxz);
    for(int hp = 0; hp < h; ++hp){
        //U分量  -179.25 -> -0.25
        fetchSegment(is, p + p1*c, p2, minz, step);
//        std::this_thread::sleep_for(10ms);
        //U分量  0.0 -> 180.
        fetchSegment(is, p, p1, minz, step);
        //next latitude
        p += w*c;
    }
}
void fetchCompentFlip(std::istream& is, std::uint8_t* p, double minz, double maxz)
{
    p += (h - 1)*w*c;
    double step = stepValue(minz, maxz);
    for(int hp = h; hp > 0; --hp){
        //-179.25 -> -0.25
        fetchSegment(is, p + p1*c, p2, minz, step);
        // 0.0 -> 180.
        fetchSegment(is, p, p1, minz, step);
        //next latitude
        p -= w*c;
    }
}
