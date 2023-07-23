#include "csv_component.h"
#include <algorithm>
#include <format>
#include <stdexcept>
#include <string>
namespace {
const int c_ = 3;                      /**< 通道数量 */
}//namespace
void CsvComponent::init(int w, int h)
{
    w_ = w;
    h_ = h;
    buff_.resize(w_*h_*c_);
}
const std::uint8_t* CsvComponent::buff() const
{
    return buff_.data();
}
int CsvComponent::size() const
{
    return buff_.size();
}
double CsvComponent::stepValue(double minz, double maxz) const
{
    return (maxz - minz)/255.;
}
std::uint8_t CsvComponent::normal(double v, double minz, double step) const
{
    return static_cast<std::uint8_t>(std::clamp((v - minz)/step, 0., 255.));
}
void CsvComponent::fetchSegment(std::istream& is, std::uint8_t* p, int count
    , double minz, double step) const
{
    std::string linestr;
    while(std::getline(is, linestr)){
        int pos = linestr.rfind(",");
        if(pos == std::string::npos){
            std::string msg = std::format("无效行:{}", linestr);
            throw std::runtime_error(msg);
        }
        std::string vstr(linestr.substr(pos + 1));
        double v = std::stod(vstr);
        *p = normal(v, minz, step);
        p += c_;
        if(--count == 0) break;
    }
}
void CsvComponent::fetchCompent(std::istream& is, int c
                                    , double minz, double maxz)
{
    if(c >= c_){
        throw std::runtime_error(std::format("通道数量大于{}", c_));
    }
    const int p1 = w_/2 + 1;  //0 -> 180
    const int p2 = w_/2 - 1;  //-179.75 -> -0.25
    std::uint8_t* p = buff_.data() + c;
    p += (h_ - 1)*w_*c_;
    const double step = stepValue(minz, maxz);
    for(int hp = h_; hp > 0; --hp){
        //-179.25 -> -0.25
        fetchSegment(is, p + p1*c_, p2, minz, step);
        // 0.0 -> 180.
        fetchSegment(is, p, p1, minz, step);
        //next latitude
        p -= w_*c_;
    }
}
