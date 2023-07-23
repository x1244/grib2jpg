/**\file
 *
 * \brief GRIB2导出的csv分量转jpg图像
 *
 * grib2jpg --flip --shift -w 1440 -h 721 -c 2 -v -23.0378,31.0522 -v -24.9615,23.7585 -u 95 -s z.csv -o uv.jpg
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <cxxopts.hpp>
#include <icecream.hpp>
#include "csv_component.h"
#include "jpeg_transfer.h"
using namespace std;
/**\brief 创建命令行参数
 */
cxxopts::Options createOptions();
int main(int argc, char** argv)
{
    bool flip = false;
    bool shift = false;
    int w, h, c;
    int qulity;
    double offset;
    string csvfile, jpgfile;
    std::vector<double> mm;
    cxxopts::Options option = createOptions();
    try{
        cxxopts::ParseResult pr = option.parse(argc, argv);
        if(pr.count("flip")) flip = true;
        if(pr.count("shift")) shift = true;
        w = pr["width"].as<int>();
        h = pr["height"].as<int>();
        c = pr["component"].as<int>();
        csvfile = pr["input"].as<string>();
        jpgfile = pr["output"].as<string>();
        qulity = pr["qulity"].as<int>();
        offset = pr["offset"].as<double>();
        mm = pr["interval"].as<vector<double> >();
    }
    catch(std::exception& ex){
        cout <<"ERROR:" <<ex.what() <<endl;
        cout <<option.help() <<endl;
        return 0;
    }
    if(mm.size() < c*2){
        cout <<"分量值域不匹配" <<endl;
        return 0;
    }

    std::ifstream is(csvfile);
    if(!is.is_open()){
        cout <<"无法打开" <<csvfile <<endl;
        return 0;
    }
    CsvComponent cc;
    cc.init(w, h, flip, shift, offset);
    try{
        for(int ci = 0; ci < c; ++ci){
            cc.fetchCompent(is, ci, mm[ci*c], mm[ci*c + 1]);
        }
    }
    catch(std::exception& ex){
        cout <<ex.what() <<endl;
        return 0;
    }
    JpegTransfer jpeg;
    try{
        jpeg.setSource(cc.buff(), cc.size(), w, h, c);
        jpeg.transferUpper();
        jpeg.save(jpgfile, qulity);
    }
    catch(std::exception& ex){
        cout <<"ERROR:" <<ex.what() <<endl;
        return 0;
    }
    cout <<"图像已保存至" <<jpgfile <<endl;
    return 0;
}
cxxopts::Options createOptions()
{
    cxxopts::Options option("grib2jpg", "GRIB2导出的csv数据转jpg");
    option.add_options()
        ("flip", "flip up and down")
        ("shift", "shift left and right")
        ("s,input", "input csv file", cxxopts::value<string>())
        ("o,output", "output jpeg file", cxxopts::value<string>())
        ("c,component", "climate element components", cxxopts::value<int>())
        ("v,interval", "min and max value intervals"
            , cxxopts::value<std::vector<double> >())
        ("u,qulity", "compress qulity", cxxopts::value<int>())
        ("w,width", "horizontal pixels", cxxopts::value<int>())
        ("h,height", "vertical pixels", cxxopts::value<int>())
        ("t,offset", "offset of the value"
            , cxxopts::value<double>()->default_value("0.0"));
    return option;
}
