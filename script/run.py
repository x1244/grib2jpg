import sys
import pathlib
import subprocess
import re
import json
#Usage: ./run.py f:/xx/v f:/xx/p f:/xx/cfg.json climate
# argv[1] - 输入文件夹
# argv[2] - 输出文件夹
# argv[3] - 配置文件输出
# argv[4] - 配置中场源文件前缀目录
def level():
    return ["20 mb", "250 mb", "300 mb", "350 mb", "400 mb",
         "450 mb", "550 mb", "600 mb", "700 mb", "800 mb",
         "900 mb", "20 m above ground", "0.995 sigma level"]
def component():
    return ["TMP", "RH", "UGRD|VGRD"]
def comstr(c):
    cs = {"TMP":"temperature", "RH":"rh", "UGRD|VGRD":"wind"}
    return cs[c]
def levelmap():
    eh = {"20 mb":23307.0, "250 mb":10355.0, "300 mb":9157.0,
          "350 mb":8111.0, "400 mb":7180.0, "450 mb":6339.0,
          "550 mb":4861.0, "600 mb":4203.0,
          "700 mb":3010.0, "800 mb":1947.0, "900 mb":987.0,
          "20 m above ground":20.0, "0.995 sigma level":3.0}
    evmp = []
    for k, v in eh.items():
        evmp.append({"name":k, "height":v})
    return evmp

def levelh(e):
    eh = {"20 mb":23307.0, "250 mb":10355.0, "300 mb":9157.0,
          "350 mb":8111.0, "400 mb":7180.0, "450 mb":6339.0,
          "550 mb":4861.0, "600 mb":4203.0,
          "700 mb":3010.0, "800 mb":1947.0, "900 mb":987.0,
          "20 m above ground":20.0, "0.995 sigma level":3.0}
    return eh[e]
class CfgItem:
    def setCv(self, c, v):
        self.c = c
        self.v = v
    def setU(self, umin, umax):
        self.umin = umin
        self.umax = umax
    def setV(self, vmin, vmax):
        self.vmin = vmin
        self.vmax = vmax
    def setFile(self, name, dt):
        self.name = name
        self.dt = dt
    def toMap(self, pxx):
        ppp = pathlib.Path(self.name)
        pbs = ppp.name
        pbs.insert(8, "T")  //time
        pbs.insert(6, "-")  //month
        pbs.insert(4, "-")  //year
        pbs.append(":00:00.000")
        if len(pxx) != 0:
            pbs = "/".join([pxx, pbs])
        mpp = {}
        if hasattr(self, 'vmin'):
            mpp = {"src": pbs, "time":self.dt,
                "umin":self.umin, "umax":self.umax,
                "vmin":self.vmin, "vmax":self.vmax,
                "left":-180.0, "right":180.0,
                "bottom":-90.0, "top":90.0,
                "level":levelh(self.v)}
        else:
            mpp = {"src": pbs, "time":self.dt,
                "umin":self.umin, "umax":self.umax,
                "left":-180.0, "right":180.0,
                "bottom":-90.0, "top":90.0,
                "level":levelh(self.v)}
        return mpp

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
        cfg = CfgItem()
        cfg.setFile(self.jpeg, self.dt)
        cfg.setCv(c, ev)
        cfg.setU(self.umin, self.umax)
        if hasattr(self, 'vmin'):
            cfg.setV(self.vmin, self.vmax)
        return cfg;

    def toJpeg(self, cmd, filename, c, ev):
        print("fetch component...")
        csv = self.genCsv(filename, c, ev)
        print("generate jpeg...")
#        subprocess.run(cmd)
        return self.realizeCfg(c, ev)
    def uvToJpeg(self, filename, c, ev):
        cmd = ["grib2jpg", "--flip", "--shift", 
                "-w", str(self.w), "-h", str(self.h),
                "-c", str(2), "-v", Jpeg.rangeStr(self.umin, self.umax),
                "-v", Jpeg.rangeStr(self.vmin, self.vmax),
                "-u", str(90), "-s", self.csvFilename(), 
                "-o", self.jpegFile(c, ev)]
        return self.toJpeg(cmd, filename, c, ev)
    def grayToJpeg(self, filename, c, ev):
        cmd = ["grib2jpg", "--flip", "--shift", 
                "-w", str(self.w), "-h", str(self.h),
                "-c", str(1), "-v", Jpeg.rangeStr(self.umin, self.umax),
                "-u", str(90), "-s", self.csvFilename(), 
                "-o", self.jpegFile(c, ev)]
        return self.toJpeg(cmd, filename, c, ev)


'''目录文件列表

   Param root: 目录
   return list: 文件列表
'''
def filelist(root):
    files = []
    for fn in pathlib.Path(root).iterdir():
        if fn.is_file():
            files.append(fn)
        elif fn.is_dir():
            files.extend(filelist(fn))
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
#返回配置
def runGrib2(filename, jpeg): 
#    cmd = "wgrib2 \"" + filename + "\""
    datetag = detectDate(filename)
    jpeg.setDateTime(datetag)
    cmd = ["wgrib2", filename]   
    cps = subprocess.run(cmd, stdout = subprocess.PIPE)
    cfg = []
    if len(cps.stdout) != 0:
        print("process", filename)
        for c in component() :
            for v in level() :
                intervals = statsComponent(filename, c, v)
                if len(intervals) == 2:
                    jpeg.setUrange(intervals[0], intervals[1])
                    ci = jpeg.grayToJpeg(filename, c, v)
                    cfg.append(ci)
                elif len(intervals) == 4:
                    jpeg.setUrange(intervals[0], intervals[1])
                    jpeg.setVrange(intervals[2], intervals[3])
                    ci = jpeg.uvToJpeg(filename, c, v)
                    cfg.append(ci)
        print("done.")
    return cfg
        #用str转bytes不好用
#        cps = subprocess.run(cmd, stdout = subprocess.PIPE)
#        lines = cps.stdout.decode('utf-8').split("\n")

def dumpCfg(cfgfile, cfg, pxx):
    ggg = {"level":levelmap()}
    for ccc in cfg:
        for ccp in ccc:
            css = comstr(ccp.c)
            cmm = ccp.toMap(pxx)
            if css in ggg:
                ggg[css].append(cmm)
            else:
                ggg[css] = [cmm]
    with open (cfgfile, 'w') as fff:
        json.dump(ggg, fff, indent=2)

def main():
    if len(sys.argv) < 4 :
        print("Usage run.py root dest cfg climate")
        return
    root = sys.argv[1]
    dest = sys.argv[2]
    cfgfile = sys.argv[3]
    jpegpxx = ""
    if len(sys.argv) > 4 :
        jpegpxx = sys.argv[4]
    if not checkDir(root, dest) :
        return

    allfiles = filelist(root)
    available = availableGrib2(allfiles)
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
    jpeg.setPixel(pixel[0], pixel[1])

    cfg = []
    try:
        for filename in available :
            ci = runGrib2(filename, jpeg)
            cfg.append(ci)
    except OSError as ex:
        print(ex)
        return
    dumpCfg(cfgfile, cfg, jpegpxx)

    csvfile = jpeg.csvFilename()
    if pathlib.Path.exists(csvfile) :
        pathlib.Path.unlink(csvfile)
        print("CLEAN csv file")

if __name__ == "__main__":
    main()
