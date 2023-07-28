# grib2jpg
fetch grib2 component to compound jpg image

提取气象上常用的GRIB2的数据分量，再按时间、高度分类，最后合成到一个个的jpg图像中，可用于数字地球上展示气象场。
典型的实现有风场和温度场。
其中用到了官方`wgrib2.exe`工具提取数据分量，用`Python`写了一个转换过程脚本，用'jpeg.lib`库来生成jpg图像。
