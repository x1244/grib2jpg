#include "csv_component.h"
#include <algorithm>
#include <format>
#include <stdexcept>
#include <string>
#include <icecream.hpp>
using namespace std;
namespace {
const int c_ = 3;                      /**< 通道数量 */
}//namespace
void CsvComponent::init(int w, int h, bool flip, bool shift, double offset)
{
    w_ = w;
    h_ = h;
    p1_ = w_/2 + 1;  //0 -> 180
    p2_ = w_/2 - 1;  //-179.75 -> -0.25
    flip_ = flip;
    shift_ = shift;
    offset_ = offset;
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
void CsvComponent::fetchSegment(istream& is, uint8_t* p, int count) const
{
    std::string linestr;
    while(std::getline(is, linestr)){
        int pos = linestr.rfind(",");
        if(pos == std::string::npos){
            std::string msg = std::format("无效行:{}", linestr);
            throw std::runtime_error(msg);
        }
        std::string vstr(linestr.substr(pos + 1));
        double v = std::stod(vstr) + offset_;
        *p = normal(v, minz_, step_);
        p += c_;
        if(--count == 0) break;
    }
}
void CsvComponent::fetchCompent(istream& is, int c, double minz, double maxz)
{
    if(c >= c_){
        throw std::runtime_error(std::format("通道数量大于{}", c_));
    }
    minz += offset_;
    maxz += offset_;
    minz_ = minz;
    step_ = stepValue(minz, maxz);
    std::uint8_t* p = buff_.data() + c;
    if(flip_){
        doFetchFlip(is, p);
    }
    else{
        doFetch(is, p);
    }
}
void CsvComponent::doFetch(istream& is, uint8_t* p)
{
    for(int hp = 0; hp < h_; ++hp){
        doSegment(is, p);
        p += w_*c_;
    }
}
void CsvComponent::doFetchFlip(istream& is, uint8_t* p)
{
    p += (h_ - 1)*w_*c_;
    for(int hp = h_; hp > 0; --hp){
        doSegment(is, p);
        p -= w_*c_;
    }
}
void CsvComponent::doSegment(std::istream& is, std::uint8_t* p)
{
    if(shift_){
        fetchSegment(is, p + p1_*c_, p2_);
        fetchSegment(is, p, p1_);
    }
    else{
        fetchSegment(is, p, p1_ + p2_);
    }
}
