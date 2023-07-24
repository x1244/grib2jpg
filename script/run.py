import sys
import pathlib
import subprocess
import re
def level():
    return ["20 mb", "250 mb", "300 mb", "350 mb", "400 mb",
         "450 mb", "550 mb", "600 mb", "700 mb", "800 mb",
         "900 mb", "20 m above ground", "0.995 sigma level"]
def component():
    return ["UGRD|VGRD", "TMP", "RH"]

#class Value:

'''目录文件列表

   Param root: 目录
   return list: 文件列表
'''
def filelist(root):
    files = []
    for fn in pathlib.Path(root).iterdir():
        if fn.is_file():
            files.append(fn)
    return files
'''检查输入输出目录是否正确

   输出目录不存在时，创建该目录

   Param root: 输入目录
   Param dest: 输出目录
   return bool: True-正确；False:不正确
'''
def checkDir(root, dest):
    if not pathlib.Path(root).is_dir() :
        print("指定位置 %s 目录不存在" %root)
        return False
    if not pathlib.Path(dest).exists() :
        try:
            pathlib.Path.mkdir(dest, parents=True)
        except OSError as e:
            print("创建目录 %s 失败" %dest)
            return False
        print("创建目录 %s" %dest)
    elif not pathlib.Path(dest).is_dir() :
        print("目录 %s 与文件同名")
        return False
    return True

def availableGrib2(files):
    av = []
    for filename in files :
        cmd = "wgrib2 " + str(filename)
        cps = subprocess.run(cmd, stdout = subprocess.PIPE)
        if len(cps.stdout) != 0:
            av.append(str(filename))
    return av

def detectDate(filename):
    cmd = "wgrib2 -v \"" + filename + "\""
    cps = subprocess.run(cmd, stdout = subprocess.PIPE)
    lines = cps.stdout.decode('utf-8').split("\n")
    for line in lines:
        res = re.search("d=(\d{10})", line)
        if res != None :
            return res.group(1)
    return None

def detectPixel(filename):
    cmd = "wgrib2 -nxny \"" + filename + "\""
    cps = subprocess.run(cmd, stdout = subprocess.PIPE)
    lines = cps.stdout.decode('utf-8').split("\n")
    for line in lines:
        res = re.search("(\d+) x (\d+)", line)
        if res != None :
            return [int(res.group(1)), int(res.group(2))]
    return None

def statsComponent(filename, cnn, ev):
    cmd = "wgrib2 -match \"" + cnn + "\" -if_fs \"" + ev + "\" -stats " + "\""+ filename + "\"";
    cps = subprocess.run(cmd, stdout = subprocess.PIPE)
    lines = cps.stdout.decode('utf-8').split("\n")
    intervals = []
    for line in lines:
        res = re.search("min=(.+):max=(.+):", line)
        if res != None :
            intervals.append(float(res.group(1)));
            intervals.append(float(res.group(2)));
    return intervals

def runGrib2(filename, datetag, width, height): 
    cmd = "wgrib2 \"" + filename + "\""
    cps = subprocess.run(cmd, stdout = subprocess.PIPE)
    if len(cps.stdout) != 0:
        for c in component() :
            for v in level() :
                intervals = statsComponent(filename, c, v)
                if len(intervals) == 2:
                    print(c, v, intervals[0], intervals[1])
                elif len(intervals) == 4:
                    print(c, v, intervals[0], intervals[1], intervals[2], intervals[3])
        #用str转bytes不好用
#        lines = cps.stdout.decode('utf-8').split("\n")
#        cmd2 = cmd + " -match TMP -
#        cpts = subprocess.run(cmd, stdout = subprocess.PIPE)

def main():
    if len(sys.argv) < 3 :
        print("Usage run.py root dest")
        return
    root = sys.argv[1]
    dest = sys.argv[2]
    if not checkDir(root, dest) :
        return

    available = availableGrib2(filelist(root))
    if len(available) == 0:
        print("未获得有效GRIB2文件")
        return

    datetag = detectDate(available[0])
    if datetag == None:
        print("未获得GRIB2文件时间标识")
        return
    pixel = detectPixel(available[0])
    print(pixel)
    if pixel == None:
        print("未获得GRIB2扫描线")
        return

    for filename in available :
        runGrib2(filename, datetag, pixel[0], pixel[1])

if __name__ == "__main__":
    main()
