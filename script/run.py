import sys
import pathlib
import subprocess
import re
import json
def level():
    return ["20 mb", "250 mb", "300 mb", "350 mb", "400 mb",
         "450 mb", "550 mb", "600 mb", "700 mb", "800 mb",
         "900 mb", "20 m above ground", "0.995 sigma level"]
def component():
    return ["TMP", "RH", "UGRD|VGRD"]
class Cfg:
    pass
class Jpeg:
    def setPixel(self, w, h):
        self.w = w
        self.h = h
    def setUrange(self, mini, maxi):
        self.umin = mini
        self.umax = maxi
    def setVrange(self, mini, maxi):
        self.vmin = mini
        self.vmax = maxi
    def setDest(self, dest):
        self.dest = dest
    def setDateTime(self, dt):
        self.dt = dt
'''
    def writeCfg(self):
        cfg = pathlib.Path(self.dest).joinpath("cfg.json")
        if cfg.is_file():
            pathlib.Path.unlink(cfg)
        elif cfg.is_dir():
            raise OSError("config file cfg.json error")
        print("confi write to file")
        with open(str(cfg), 'w') as cfgfile:
            for c in self.cfg:
                cfgfile.write(c.umin, c.umax)
#                if c.vmin != NONE:
#                    cfgfile.write(c.vmin, c.vmax)
'''
    def jpegFile(self, c, ev):
        ev = ev.replace(" ", "_")
        ev = ev.replace(".", "p")
        part = [self.dt, c.replace("|", "_"), ev]
        ppp = pathlib.Path(self.dest).joinpath("_".join(part) + ".jpg")
        self.jpeg = str(ppp)
        return self.jpeg
    def csvFilename(self):
        csv = pathlib.Path(self.dest).joinpath("cmp.csv")
        return str(csv)
    def genCsv(self, filename, c, ev):
        csv = pathlib.Path(self.dest).joinpath("cmp.csv")
        if csv.is_file():
            pathlib.Path.unlink(csv)
        elif csv.is_dir():
            raise OSError("CSV file cmp.csv error")
        csvfile = str(csv)
        cmd = ["wgrib2", "-match", c, "-if_fs", ev, "-csv", csvfile, filename]
        subprocess.run(cmd, stdout = subprocess.PIPE)
        return csvfile
    def rangeStr(mini, maxi):
        return ",".join([str(mini), str(maxi)])
    def realizeCfg(self, c, ev):
        cfg = Cfg()
        cfg.datetag = self.dt
        cfg.name = self.jpeg
        cfg.umin = self.umin
        cfg.umax = self.umax
#        cfg.vmin = self.vmin
#        cfg.vmax = self.vmax

    def toJpeg(self, cmd, filename, c, ev):
        print("fetch component...")
        csv = self.genCsv(filename, c, ev)
        print("generate jpeg...")
        subprocess.run(cmd)
        self.realizeCfg(c, ev)
    def uvToJpeg(self, filename, c, ev):
        cmd = ["grib2jpg", "--flip", "--shift", 
                "-w", str(self.w), "-h", str(self.h),
                "-c", str(2), "-v", Jpeg.rangeStr(self.umin, self.umax),
                "-v", Jpeg.rangeStr(self.vmin, self.vmax),
                "-u", str(90), "-s", self.csvFilename(), 
                "-o", self.jpegFile(c, ev)]
        self.toJpeg(cmd, filename, c, ev)
    def grayToJpeg(self, filename, c, ev):
        cmd = ["grib2jpg", "--flip", "--shift", 
                "-w", str(self.w), "-h", str(self.h),
                "-c", str(1), "-v", Jpeg.rangeStr(self.umin, self.umax),
                "-u", str(90), "-s", self.csvFilename(), 
                "-o", self.jpegFile(c, ev)]
        self.toJpeg(cmd, filename, c, ev)


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
        cmd = ["wgrib2", str(filename)]
        cps = subprocess.run(cmd, stdout = subprocess.PIPE)
        if len(cps.stdout) != 0:
            av.append(str(filename))
    return av

def detectDate(filename):
    cmd = ["wgrib2", "-v", filename]
    cps = subprocess.run(cmd, stdout = subprocess.PIPE, encoding = "utf-8")
    lines = cps.stdout.split("\n")
    for line in lines:
        res = re.search("d=(\d{10})", line)
        if res != None :
            return res.group(1)
    return None

def detectPixel(filename):
    cmd = ["wgrib2", "-nxny", filename]
    cps = subprocess.run(cmd, stdout = subprocess.PIPE, encoding = "utf-8")
    lines = cps.stdout.split("\n")
    for line in lines:
        res = re.search("(\d+) x (\d+)", line)
        if res != None :
            return [int(res.group(1)), int(res.group(2))]
    return None

def statsComponent(filename, cnn, ev):
    cmd = ["wgrib2", "-match", cnn, "-if_fs", ev, "-stats", filename]
    cps = subprocess.run(cmd, stdout = subprocess.PIPE, encoding = "utf-8")
    lines = cps.stdout.split("\n")
    intervals = []
    for line in lines:
        res = re.search("min=(.+):max=(.+):", line)
        if res != None :
            intervals.append(float(res.group(1)));
            intervals.append(float(res.group(2)));
    return intervals

def runGrib2(filename, jpeg): 
#    cmd = "wgrib2 \"" + filename + "\""
    cmd = ["wgrib2", filename]   
    cps = subprocess.run(cmd, stdout = subprocess.PIPE)
    if len(cps.stdout) != 0:
        print("process", filename)
        for c in component() :
            for v in level() :
                intervals = statsComponent(filename, c, v)
                if len(intervals) == 2:
                    jpeg.setUrange(intervals[0], intervals[1])
                    jpeg.grayToJpeg(filename, c, v)
                elif len(intervals) == 4:
                    jpeg.setUrange(intervals[0], intervals[1])
                    jpeg.setVrange(intervals[2], intervals[3])
                    jpeg.uvToJpeg(filename, c, v)
        print("done.")
        #用str转bytes不好用
#        cps = subprocess.run(cmd, stdout = subprocess.PIPE)
#        lines = cps.stdout.decode('utf-8').split("\n")

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
    if pixel == None:
        print("未获得GRIB2扫描线")
        return

    jpeg = Jpeg()
    jpeg.setDest(dest)
    jpeg.setDateTime(datetag)
    jpeg.setPixel(pixel[0], pixel[1])

    try:
        for filename in available :
            runGrib2(filename, jpeg)
    except OSError as ex:
        print(ex)
        return

if __name__ == "__main__":
    main()
