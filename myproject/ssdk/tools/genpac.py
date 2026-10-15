# coding=UTF-8
#!/usr/bin/python
import json
import sys
import getopt
import os
import glob
import shutil
import distutils.dir_util
import subprocess
import shlex
import math
import platform

sys.path.append(os.path.join(os.path.dirname(__file__), "SSDKDS", "scripts",""))
from Kconfiglib.sdkconfig import SDKconfig

global BPT_SZ
BPT_SZ=0x1000
global XSPI1_MEM_BASE
g_objEncryption=[]
DEF_BOOT_SZ=0x400000
PAC_ALIGN=0x20000

XSPI1_MEM_BASE = 0x10000000
def str_trans(input):
    if input == "true":
        return "1"
    else:
        return "0"


def file_align(file_name, align_size):
    try:
        file_size = math.ceil(os.path.getsize(file_name) / align_size) * align_size
        fo = open(file_name, "r+")
        fo.truncate(file_size)
        fo.close()
        print("%s is aligned to 0x%x" %(file_name, align_size))
    except Exception as e:
        raise Exception("getsize %s error:\n    %s" %(file_name,e))

def pac_align(mem_offset, align_size):
    return math.ceil(mem_offset / align_size) * align_size

def calc_dlp(mem_offset):
    return int(mem_offset/512)

def try_copy(file, path):
    try:
        shutil.copy(file, path)
    except Exception as e:
        raise Exception("copy %s error:\n    %s" %(file,e))

def exe_pac(sf_only_mode, pac_type, BOARD, PROJECT, partition_mode, xip_mode, sec_ver, \
iib_str_list, sfs_name, boot0_addr, boot1_addr, boot2_addr, rfd_name, partition_file, fat32_file, key_str, boot_package_num, ctl_str, uuid_str):

    global g_objEncryption
    print("exe_pac_start")
    dloader_unsigned = ("dloader_%s.bin" % BOARD)
    dloader_signed = ("dloader_%s_signed.bin" % BOARD)
    output_pac = os.path.join(PAC_TEMP_PATH, ("%s_%s.pac" % (pac_type, BOARD)))
    #output_pac = os.path.join("temp", ("%s_%s.pac" % (pac_type, BOARD)))

    #clean the env
    if os.path.exists(output_pac):
        os.remove(output_pac)

    os.chdir(os.path.join(os.getcwd(),"tools", "sdtools"))
    #pac_cmd = "bash %s %s %s %s %s %s %s \"%s\" %s %s %s %s %s %s %s" \
    #%(sf_only_mode, pac_type, BOARD, PROJECT, partition_mode, xip_mode, \
    #sec_ver, iib_str, sfs_name, boot0_addr, boot1_addr, boot2_addr, rfd_name, partition_file,fat32_file)

    #print(pac_cmd)
    #os.system(pac_cmd)
    #return

    print("\n-----sign dloader key = %s:" % key_str)
    try:
        sign_dloader = ATB_SIGNER + " sign --v 2 --sec_ver %s --dgst sha256 %s \n--iib core=0 type=0x0 image=temp/%s to=0x404000 entry=0x404000 %s %s \n--psn 0x01 --of temp/%s" %(sec_ver, key_str, dloader_unsigned, ctl_str, uuid_str, dloader_signed)
        print(sign_dloader)
        args = shlex.split(sign_dloader)
        subprocess.run(args, check=True)
    except Exception as e:
        raise Exception("sign dloader error:\n    %s" %e)

    ssdk_name=["ssdk_signed_normal.bin", "ssdk_signed_backup.bin", "ssdk_signed_third.bin"]
    boot_package_version = ["0x100", "0x100", "0x100"]
    num = [0, 1, 2]
    for i in num:
        if i >= boot_package_num:
            break
        print("\n-----sign boot package %d key = %s:" % (num[i], key_str))
        sign_boot_package = ATB_SIGNER + " sign --v 2 --sec_ver %s --dgst sha256 %s \n%s --psn %s --of temp/%s" %(sec_ver, key_str, iib_str_list[i], boot_package_version[i], ssdk_name[i])
        print(sign_boot_package)
        try:
            args = shlex.split(sign_boot_package)
            subprocess.run(args, check=True)
            #Encrypte boot image
            if len(g_objEncryption) > 0 and \
                    g_objEncryption["encryption_on"] == "true" and \
                    i < len(g_objEncryption["rfd"]):

                strOriginalFile = ssdk_name[i]
                strEncryFile = ssdk_name[i] + ".sign"
                strEncrypteCmd = ATB_SIGNER + " xspi_enc "

                strKs = ""
                if "ks" in g_objEncryption:
                    strKs = g_objEncryption["ks"]
                    strEncrypteCmd += " ks={0} ".format(strKs)

                strPvkName = ""
                pvkName = ["pvk0", "pvk1", "pvk2", "pvk3"]
                for pvk in pvkName:
                    if pvk in g_objEncryption["rfd"][i]:
                        strPvkName = pvk
                        break

                strKek = ""
                if "kek" in g_objEncryption["rfd"][i]:
                    strKek = g_objEncryption["rfd"][i]["kek"]

                strIek = ""
                if "iek" in g_objEncryption["rfd"][i]:
                    strIek = g_objEncryption["rfd"][i]["iek"]

                strIv = g_objEncryption["rfd"][i]["iv"]

                if strPvkName != "":
                    strPvkVal = g_objEncryption["rfd"][i][strPvkName]
                    strEncrypteCmd = strEncrypteCmd + " {0}={1} iv={2} " \
                            .format(strPvkName,strPvkVal,strIv)
                else:
                    strEncrypteCmd = strEncrypteCmd + " iek={0} iv={1} " \
                            .format(strIek,strIv)
                pass
                strEncrypteCmd = strEncrypteCmd + " if=temp/{0} of=temp/{1}" \
                        .format(strOriginalFile , strEncryFile)

                print("Encryte Boot Image...")
                print(strEncrypteCmd)
                args = shlex.split(strEncrypteCmd)
                subprocess.run(args, check=True)
                ssdk_name[i] = strEncryFile
            file_align( ("temp/%s" %ssdk_name[i]), 4096)
        except Exception as e:
            raise Exception("sign boot package error:\n    %s" %e)

    #gen sfs binary
    if pac_type == "ospi":
        print("\n-----gen_sfs_binary:")
        gen_sfs_binary = PYTHON + " pactool/gen_sfs_binary.py --json sfs/%s \n--out temp/sfs.img \n--normal_img_off %s \n--backup_img_off %s \n--third_img_off %s" %(sfs_name, boot0_addr, boot1_addr, boot2_addr)
        print(gen_sfs_binary)
        try:
            args = shlex.split(gen_sfs_binary)
            subprocess.run(args, check=True)
        except Exception as e:
            raise Exception("gen_sfs_binary error:\n    %s" %e)


    #gen pac
    if partition_mode == "0":
        print("\n-----generating PAC  without parition : %s" % output_pac)
        if pac_type == "ospi":
            paccmd = PYTHON + " pactool/pactool make_pac_image_no_gpt \n" + \
            ("--output pac/%s/%s/%s_%s.pac \n" % (BOARD, PROJECT, pac_type, BOARD)) + \
            ("--allow_empty_partitions \n") + \
            ("--preload sfs:temp/sfs.img \n") + \
            ("--preload rfd:%s \n" % rfd_name) + \
            ("--preload boot0:temp/%s \n" % ssdk_name[0]) + \
            ("--da FDA:temp/%s \n" % dloader_signed)

        elif pac_type == "emmc":
            paccmd = PYTHON + " pactool/pactool make_pac_image_no_gpt \n" + \
            ("--output pac/%s/%s/%s_%s.pac \n" % (BOARD, PROJECT, pac_type, BOARD)) + \
            ("--allow_empty_partitions \n") + \
            ("--preload boot0:temp/%s \n" % ssdk_name[0]) + \
            ("--da FDA:temp/%s \n" % dloader_signed)

        if boot_package_num > 1:
            paccmd = paccmd +("--preload boot1:temp/%s \n" % ssdk_name[1])

        if boot_package_num > 2:
            paccmd = paccmd +("--preload boot2:temp/%s \n" % ssdk_name[2])

        try:
            args = shlex.split(paccmd)
            print(paccmd)
            subprocess.run(args, check=True)
        except Exception as e:
            raise Exception("generating PAC  without parition error:\n    %s" %e)

    else:
        print("\n-----generating PAC with parition : %s" % output_pac)

        bptcmd=  PYTHON + " pactool/bpttool make_table \n"   + \
                 ("--input bpt/%s --ab_suffixes \"_a,_b\" \n"  % partition_file) + \
                 "--output_json temp/output.bpt \n"  + \
                 "--output_gpt temp/partition.img \n"

        if pac_type == "ospi":
            paccmd = PYTHON + " pactool/pactool make_pac_image \n" + \
            ("--input temp/output.bpt \n") + \
            ("--output pac/%s/%s/%s_%s.pac \n" % (BOARD, PROJECT, pac_type, BOARD)) + \
            ("--image boot0:temp/%s \n" % ssdk_name[0]) +\
            ("--allow_empty_partitions \n") +\
            ("--preload sfs:temp/sfs.img \n") +\
            ("--preload rfd:%s \n" % rfd_name) +\
            ("--da FDA:temp/%s \n" % dloader_signed)

            if boot_package_num > 1:
                paccmd = paccmd + ("--image boot1:temp/%s \n" % ssdk_name[1])

            if boot_package_num > 2:
                paccmd = paccmd + ("--image boot2:temp/%s \n" % ssdk_name[2])

        else:
            paccmd = PYTHON + " pactool/pactool make_pac_image \n"  + \
            ("--input temp/output.bpt \n")  + \
            ("--output pac/%s/%s/%s_%s.pac \n" % (BOARD, PROJECT, pac_type, BOARD)) + \
            ("--preload boot0:temp/%s \n" % ssdk_name[0]) + \
            ("--allow_empty_partitions \n") + \
            ("--da FDA:temp/%s \n" % dloader_signed)

            if boot_package_num > 1:
                paccmd = paccmd + ("--preload boot1:temp/%s \n" % ssdk_name[1])

            if boot_package_num > 2:
                paccmd = paccmd + ("--image boot2:temp/%s \n" % ssdk_name[2])

        try:
            b = open(os.path.join(SDTOOLS, "bpt", partition_file))
            bptfile = json.load(b)
            partitions = bptfile["partitions"]
            for pt_inst in partitions :
                label = pt_inst["label"]
                if label == "boot0" or label == "boot1" or label == "boot2":
                    continue
                elif label == "res":
                    paccmd = paccmd + "--image res:temp/%s \n" %(fat32_file)
                else:
                    for files in glob.glob(os.path.join(TEMP_PATH, "*")):
                        name_temp = os.path.basename(files)
                        if name_temp == label:
                            paccmd = paccmd + "--image %s:temp/%s \n" % (label,label)
                        elif name_temp == (label +".bin"):
                            paccmd = paccmd + "--image %s:temp/%s.bin \n" % (label,label)
                        elif name_temp == (label +".img"):
                            paccmd = paccmd + "--image %s:temp/%s.img \n" % (label,label)

        except Exception as e:
            raise Exception("read temp/output.bpt error: %s", e)

        try:
            args = shlex.split(bptcmd)
            print(bptcmd)
            subprocess.run(args, check=True)
        except Exception as e:
            raise Exception("make parition table error:\n    %s" %e)

        try:
            args = shlex.split(paccmd)
            print(paccmd)
            subprocess.run(args, check=True)
        except Exception as e:
            raise Exception("generating PAC with parition error:\n    %s" %e)

    os.chdir(PWD)
    try_copy(os.path.join(PAC_TEMP_PATH, "%s_%s.pac" % (pac_type, BOARD)), PAC_PTAH)
    print("exe_pac_end")

kconfgDict = {}
def read_kconfig(board, core, project, cfg):

    try:
        rlt_str = kconfgDict[os.path.join(board,core,project,cfg)]
    except:
        CORE_VAL = os.getenv("CORE")
        os.environ['CORE'] = core

        kconfig_path  = os.path.join(os.getcwd())
        project_path = os.path.join(os.getcwd(), "boards",board,core,"projects",project)
        kconfig = SDKconfig(project_path,kconfig_path,project_path)
        rlt_str = kconfig.get_symb(cfg)

        try:
            os.environ['CORE'] = CORE_VAL
        except:
            os.environ['CORE'] = core

        kconfgDict[os.path.join(board,core,project,cfg)] = rlt_str
        print("read " + board + " " + core + " " + project + " " + cfg + " as " + rlt_str)

    return rlt_str

def gen_pac(board_name, project_name):

    global g_objEncryption
    print("Generate PAC according to %s" % pac_json)

    try:
        f = open(pac_json)
    except:
        raise("Can not find %s" % pac_json)

    json_cfg = json.load(f)
    description = json_cfg.get("description", "e3 pac json file")
    print("description is " + description)

    if "pac" in json_cfg:
        pac = json_cfg.get("pac")
        for pac_inst in pac :
            shutil.rmtree(TEMP_PATH, ignore_errors=True)
            shutil.rmtree(BIN_PATH, ignore_errors=True)
            os.makedirs(PAC_TEMP_PATH, exist_ok=True)
            os.makedirs(TEMP_PATH, exist_ok=True)
            os.makedirs(BIN_PATH, exist_ok=True)

            # copy core binaries to bin dir
            for bin in glob.glob(ssdk_bin_file):
                try_copy(bin, BIN_PATH)

            if QUERY_MODE == 0:
                # copy dloader binary
                try_copy(dloader, os.path.join(BIN_PATH, ("dloader_%s.bin" % board_name)))

                # copy FAT image
                for fat in glob.glob(fat_file):
                    try_copy(fat, BIN_PATH)

                # copy other binaries
                for other in glob.glob(other_file):
                    try_copy(other, BIN_PATH)

            # copy bin dir to temp dir
            for files in glob.glob(os.path.join(BIN_PATH, "*")):
                try_copy(files, TEMP_PATH)

            try:
                pac_type = pac_inst["type"]
            except:
                raise Exception("can not find [pac]-[type] in [%s] please check json file" %pac_json)
            print("\n****************************************gen pac for %s************************************************\n " % pac_type)
            try:
                sfs = pac_inst["sfs"]
                sfs_name = sfs["name"]
                boot0_addr = sfs["boot0_addr"]
                boot1_addr = sfs["boot1_addr"]
                boot2_addr = sfs["boot2_addr"]
            except:
                if pac_type == "ospi":
                    raise Exception("Error in [%s] 's sfs, please check json file"  % pac_json)
                else:
                    sfs_name = "mt35-1-1-1.json"
                    boot0_addr = "0x7000"
                    boot1_addr = "0x207000"
                    boot2_addr = "0x407000"

            boot0_end = 0x0
            boot1_end = 0x0
            boot2_end = 0x0

            if boot1_addr == "":
                boot0_end = int(boot0_addr,16) + int(DEF_BOOT_SZ,16)
            else:
                boot0_end = int(boot1_addr,16)

            if boot2_addr == "":
                boot1_end = int(boot1_addr,16) + int(boot1_addr, 16) - int(boot0_addr)
            else:
                boot1_end = int(boot2_addr,16)
                boot2_end = int(boot2_addr, 16) + int(boot2_addr, 16) - int(boot1_addr,16)

            boot_region = [{"start":int(boot0_addr,16), "end":boot0_end},{"start":int(boot1_addr,16), "end":boot1_end},{"start":int(boot2_addr,16), "end":boot2_end}]
            #Read Encryption configuration
            strNewRfd=""
            if "encryption" in pac_inst:
                g_objEncryption = pac_inst["encryption"]
                print("Begin to creating rfd image..."+g_objEncryption["encryption_on"])
                if g_objEncryption["encryption_on"] == "true":
                    print("Begin to creating rfd image...")
                    strRfdCmd = ""
                    strSrkKey = ""
                    if "srk" in g_objEncryption:
                        strSrkKey = g_objEncryption["srk"]

                    strKs = ""
                    if "ks" in g_objEncryption:
                        strKs = g_objEncryption["ks"]

                    objRfdArray = g_objEncryption["rfd"]

                    strRfdCmd = " xspi_wrap "
                    if strSrkKey != "":
                        strRfdCmd += " srk=" + strSrkKey

                    if strKs != "":
                        strRfdCmd += " ks=" + strKs

                    print("strRfdCmd:"+strRfdCmd)
                    rfdIdx = 0
                    for objRfd in objRfdArray:
                        strCurRfdCmd=""
                        strKek = ""
                        if "kek" in objRfd:
                            strKek = objRfd["kek"]

                        strIek = ""
                        if "iek" in objRfd:
                            strIek = objRfd["iek"]

                        strIv = objRfd["iv"]
                        strStart = objRfd["start"]
                        strEnd = objRfd["end"]
                        strPvk = ""
                        pvkName = ["pvk0", "pvk1", "pvk2", "pvk3"]
                        for pvk in pvkName:
                            if pvk in objRfd:
                                strPvk = pvk
                                break

                        if "boot" in objRfd and objRfd["boot"] == "true":
                            if boot_region[rfdIdx]["start"] != int(strStart, 16) or boot_region[rfdIdx]["end"] != int(strEnd, 16):
                                print("rfd:"+str(rfdIdx)+" region addr is not match sfs!")
                                print("rfd start:" + strStart + " rfd end:"+ strEnd)
                                print("boot start:%#x"%boot_region[rfdIdx]["start"] + " boot end:%#x"%boot_region[rfdIdx]["end"])
                                exit()


                        if strPvk != "":
                            strPvkVal = objRfd[strPvk]
                            strCurRfdCmd = " --rfd {0}={1} iv={2} start={3} end={4} "\
                                    .format(strPvk,strPvkVal,strIv,strStart,strEnd)
                            pass
                        else:
                            strCurRfdCmd = " --rfd kek={0} iek={1} iv={2} start={3} end={4} "\
                                    .format(strKek,strIek,strIv,strStart,strEnd)
                            pass
                        rfdIdx += 1
                        strRfdCmd = strRfdCmd + strCurRfdCmd
                    strNewRfd="temp/new.rfd"
                    os.chdir(os.path.join(os.getcwd(),"tools", "sdtools"))
                    strRfdCmd = ATB_SIGNER  + strRfdCmd + " of=" + strNewRfd
                    print("Generate RFD Command:")
                    print(strRfdCmd)
                    args = shlex.split(strRfdCmd)
                    subprocess.run(args, check=True)
                    os.chdir(PWD)
            else:
                print("No Encryption configuration,Skip...")
                g_objEncryption = []


            xip_mode       = str_trans(pac_inst.get("xip_mode", "false"))
            partition_mode = str_trans(pac_inst.get("partition", "false"))
            sf_only_mode   = str_trans(pac_inst.get("sf_only_mode", "false"))
            partition_file = pac_inst.get("partition_file", "none")
            sec_ver       = pac_inst.get("sec_ver", "0")
            rfd_name      = pac_inst.get("rfd_name", "xspi_wrap_rfd_blk_pvk1_ks.rfd")
            fat32_file    = pac_inst.get("fat32_file", "fat.img")
            key           = pac_inst.get("key", "none")
            ctl           = pac_inst.get("ctl", "none")
            uuid           = pac_inst.get("uuid", "none")

            if strNewRfd == "":
                rfd_name = "rfd/" + rfd_name
            else:
                rfd_name = strNewRfd

            if key != "none":
                key_str = "--rcp key=sign_tool/keys/%s" % key
            else:
                key_str = " "

            if ctl != "none":
                ctl_str = "ctl=%s" % ctl
            else:
                ctl_str = " "

            if uuid != "none":
                uuid_str = "uuid=%s" % uuid
            else:
                uuid_str = " "

            boot_package_num    = int(pac_inst.get("boot_package_num", "3"))

            if (boot_package_num > 3) or (boot_package_num <= 0):
                boot_package_num = 3

            if boot_package_num == 1:
                boot1_addr = "0x0"
                boot2_addr = "0x0"
            if boot_package_num == 2:
                boot2_addr = "0x0"

            if pac_type != "ospi":
                xip_mode = "0"

            if sf_only_mode == "1":
                partition_mode = "1"
                if partition_file == "none":
                    raise Exception("partition_file is none in [%s] please check json file" %pac_json)

            try:
                core = pac_inst["core"]
                iib_str_list = []
                bootloader_mode = str_trans(core.get("bootloader_mode", "false"))
                if bootloader_mode == "1":
                    try_copy(bootloader, os.path.join(BIN_PATH,  "bootloader.bin"))
                    try_copy(bootloader, os.path.join(TEMP_PATH, "bootloader.bin"))
                if QUERY_MODE == 0:
                    SF_ALIGN = int(core.get("sf_pad_size", "0x1000"),16)
                    SP0_ALIGN = int(core.get("sp0_pad_size", "0x1000"),16)
                    SP1_ALIGN = int(core.get("sp1_pad_size", "0x1000"),16)
                    SX0_ALIGN = int(core.get("sx0_pad_size", "0x1000"),16)
                    SX1_ALIGN = int(core.get("sx1_pad_size", "0x1000"),16)
                    BOOTLOADER_ALIGN = int(core.get("bootloader_pad_size", "0x1000"),16)
                    #pad the bin
                    for files in glob.glob(os.path.join(TEMP_PATH, "*")):
                        if os.path.basename(files) == "sf.bin" and SF_ALIGN!= 0:
                            file_align(files, SF_ALIGN)
                        elif os.path.basename(files) == "sp0.bin" and SP0_ALIGN!= 0:
                            file_align(files, SP0_ALIGN)
                        elif os.path.basename(files) == "sp1.bin" and SP1_ALIGN!= 0:
                            file_align(files, SP1_ALIGN)
                        elif os.path.basename(files) == "sx0.bin" and SX0_ALIGN!= 0:
                            file_align(files, SX0_ALIGN)
                        elif os.path.basename(files) == "sx1.bin" and SX1_ALIGN!= 0:
                            file_align(files, SX1_ALIGN)
                        elif os.path.basename(files) == "bootloader.bin" and BOOTLOADER_ALIGN!= 0:
                            file_align(files, BOOTLOADER_ALIGN)

                num = [0, 1, 2]
                membase = 0
                memsize = 0
                pacoffset = 0
                dlp = 0

                if pac_type == "ospi":
                    boot_addr_array = [int(boot0_addr,16), int(boot1_addr,16), int(boot2_addr,16)]

                for i in num:
                    # sf iib str

                    if i >= boot_package_num:
                        break

                    if os.path.exists(os.path.join(TEMP_PATH, "sf.bin")) == False:
                        if QUERY_MODE == 1:
                            sys.exit(1)
                        raise Exception("can not find sf.bin in %s" % os.path.join(TEMP_PATH, "sf.bin"))

                    pacoffset = BPT_SZ
                    if xip_mode == "0":
                        membase = int(read_kconfig(board_name,"sf",project_name,"MEM_BASE"), 16)
                    else:
                        xipbase = XSPI1_MEM_BASE + boot_addr_array[i]
                        membase = xipbase + pacoffset

                    memsize = os.path.getsize(os.path.join(TEMP_PATH, "sf.bin"))
                    dlp = calc_dlp(pacoffset)
                    pacoffset = pac_align((pacoffset + memsize), PAC_ALIGN)

                    iib_str = "--iib core=0 type=0x0 image=temp/sf.bin dlp=0x%x to=0x%x entry=0x%x %s %s \n" % (dlp, membase, membase, ctl_str, uuid_str)
                    if sf_only_mode == "0":
                        if read_kconfig(board_name,"sf",project_name,"PART_SX") == 'n':
                            core_sx_mode = "None"
                        else:
                            if read_kconfig(board_name,"sf",project_name,"SX_Lockstep") == 'y':
                                core_sx_mode = "lockstep"
                            elif read_kconfig(board_name,"sf",project_name,"SX_Split") == 'y':
                                core_sx_mode = "split"
                            else:
                                core_sx_mode = "None"
                        if read_kconfig(board_name,"sf",project_name,"PART_SP") == 'n':
                            core_sp_mode = "None"
                        else:
                            if read_kconfig(board_name,"sf",project_name,"SP_Lockstep") == 'y':
                                core_sp_mode = "lockstep"
                            elif read_kconfig(board_name,"sf",project_name,"SP_Split") == 'y':
                                core_sp_mode = "split"
                            else:
                                core_sp_mode = "None"

                        if core_sp_mode == "split" or core_sp_mode == "lockstep":

                            if os.path.exists(os.path.join(TEMP_PATH, "sp0.bin")) == False:
                                if QUERY_MODE == 1:
                                    print("can't find sp0.bin\n")
                                    sys.exit(1)
                                raise Exception("can not find sp0.bin in %s" % os.path.join(TEMP_PATH, "sp0.bin"))

                            if xip_mode == "0":
                                membase = int(read_kconfig(board_name,"sp0",project_name,"MEM_BASE"), 16)
                            else:
                                membase = xipbase + pacoffset

                            memsize = os.path.getsize(os.path.join(TEMP_PATH, "sp0.bin"))
                            dlp = calc_dlp(pacoffset)
                            pacoffset = pac_align((pacoffset + memsize), PAC_ALIGN)

                            if core_sp_mode == "lockstep":
                                iib_str += "--iib core=7 type=0x0 image=temp/sp0.bin dlp=0x%x to=0x%x entry=0x%x \n" % (dlp, membase, membase)
                            else:
                                iib_str += "--iib core=2 type=0x0 image=temp/sp0.bin dlp=0x%x to=0x%x entry=0x%x \n" % (dlp, membase, membase)

                                if os.path.exists(os.path.join(TEMP_PATH, "sp1.bin")) == False:
                                    if QUERY_MODE == 1:
                                        print("can't find sp1.bin\n")
                                        sys.exit(1)
                                    raise Exception("can not find sp1.bin in %s" % os.path.join(TEMP_PATH, "sp1.bin"))

                                if xip_mode == "0":
                                    membase = int(read_kconfig(board_name,"sp1",project_name,"MEM_BASE"),16)
                                else:
                                    membase = xipbase + pacoffset

                                memsize = os.path.getsize(os.path.join(TEMP_PATH, "sp1.bin"))
                                dlp = calc_dlp(pacoffset)
                                pacoffset = pac_align((pacoffset + memsize), PAC_ALIGN)

                                iib_str += "--iib core=3 type=0x0 image=temp/sp1.bin dlp=0x%x to=0x%x entry=0x%x \n" % (dlp, membase, membase)

                        if core_sx_mode == "split" or core_sx_mode == "lockstep":

                            if os.path.exists(os.path.join(TEMP_PATH, "sx0.bin")) == False:
                                if QUERY_MODE == 1:
                                    print("can't find sx0.bin\n")
                                    sys.exit(1)
                                raise Exception("can not find sx0.bin in %s" % os.path.join(TEMP_PATH, "sx0.bin"))

                            if xip_mode == "0":
                                membase = int(read_kconfig(board_name,"sx0",project_name,"MEM_BASE"),16)
                            else:
                                membase = xipbase + pacoffset

                            memsize = os.path.getsize(os.path.join(TEMP_PATH, "sx0.bin"))
                            dlp = calc_dlp(pacoffset)
                            pacoffset = pac_align((pacoffset + memsize), PAC_ALIGN)

                            if core_sx_mode == "lockstep":
                                iib_str += "--iib core=6 type=0x0 image=temp/sx0.bin dlp=0x%x to=0x%x entry=0x%x \n" % (dlp, membase, membase)

                            else:
                                iib_str += "--iib core=4 type=0x0 image=temp/sx0.bin dlp=0x%x to=0x%x entry=0x%x \n" % (dlp, membase, membase)

                                if os.path.exists(os.path.join(TEMP_PATH, "sx1.bin")) == False:
                                    if QUERY_MODE == 1:
                                        print("can't find sx1.bin\n")
                                        sys.exit(1)
                                    raise Exception("can not find sx1.bin in %s" % os.path.join(TEMP_PATH, "sx1.bin"))

                                if xip_mode == "0":
                                    membase = int(read_kconfig(board_name,"sx1",project_name,"MEM_BASE"),16)
                                else:
                                    membase = xipbase + pacoffset

                                memsize = os.path.getsize(os.path.join(TEMP_PATH, "sx1.bin"))
                                dlp = calc_dlp(pacoffset)
                                pacoffset = pac_align((pacoffset + memsize), PAC_ALIGN)

                                iib_str += "--iib core=5 type=0x0 image=temp/sx1.bin dlp=0x%x to=0x%x entry=0x%x \n" % (dlp, membase, membase)
                    elif QUERY_MODE == 1:
                        try:
                            b = open(os.path.join(SDTOOLS, "bpt", partition_file))
                            bptfile = json.load(b)
                            partitions = bptfile["partitions"]
                            for pt_inst in partitions :
                                label = pt_inst["label"]
                                if (label == "sp0") and (os.path.exists(os.path.join(TEMP_PATH, "sp0.bin")) == False):
                                    print("can't find sp0.bin\n")
                                    sys.exit(1)
                                if (label == "sp1") and (os.path.exists(os.path.join(TEMP_PATH, "sp1.bin")) == False):
                                    print("can't find sp1.bin\n")
                                    sys.exit(1)
                                if (label == "sx0") and (os.path.exists(os.path.join(TEMP_PATH, "sx0.bin")) == False):
                                    print("can't find sx0.bin\n")
                                    sys.exit(1)
                                if (label == "sx1") and (os.path.exists(os.path.join(TEMP_PATH, "sx1.bin")) == False):
                                    print("can't find sx1.bin\n")
                                    sys.exit(1)
                        except Exception as e:
                            raise Exception("qurey mode read partition bpt file error: %s", e)

                    if bootloader_mode == "1":
                        if os.path.exists(os.path.join(TEMP_PATH, "bootloader.bin")) == False :
                            if QUERY_MODE == 1:
                                sys.exit(1)
                            raise Exception("can not find bootloader.bin in %s" % os.path.join(TEMP_PATH, "bootloader.bin"))
                        membase = int(read_kconfig(board_name,"sf","bootloader","MEM_BASE"),16)
                        dlp = calc_dlp(pacoffset)
                        iib_str += "--iib core=0 type=0xf image=temp/bootloader.bin dlp=0x%x to=0x%x entry=0x%x \n" % (dlp, membase, membase)

                    iib_str_list.append(iib_str)
                    #print(iib_str_list[i])

            except Exception as e:
                raise Exception("parse project:[%s] 's core setup error:\n    %s" % (project_name, e))

            try:
                if QUERY_MODE == 0:
                    exe_pac(sf_only_mode, pac_type , board_name, project_name, partition_mode, xip_mode, sec_ver, iib_str_list, sfs_name, \
                    boot0_addr, boot1_addr, boot2_addr, rfd_name, partition_file, fat32_file, key_str, boot_package_num, ctl_str, uuid_str)

            except Exception as e:
                raise Exception("exe_pac error:\n    %s"  %e)
    else:
        raise Exception("can not find 'pac' in json, please check")


if __name__ == "__main__":
    QUERY_MODE = 0
    try:
        opts, args = getopt.getopt(sys.argv[1:], '-b:-p:-v:-q',[])
        for opt_name, opt_value in opts:
            if opt_name in ('-b', '--board'):
                BOARD = opt_value
                os.environ['BOARD'] = opt_value
                print("BOARD is %s" % BOARD)
            if opt_name in ('-p', '--project'):
                PROJECT = opt_value
                os.environ['PROJECT'] = opt_value
                print("PROJECT is %s" % PROJECT)
            if opt_name in ('-v', '--version'):
                TOOLCHAIN = opt_value
                print("TOOLCHAIN is %s" % TOOLCHAIN)
            if opt_name in ('-q', '--query'):
                QUERY_MODE = 1

        print("gen pac for board=%s, project=%s toolchain=%s" % (BOARD, PROJECT, TOOLCHAIN))
        print("QUERY_MODE=%d" % QUERY_MODE)

        PAC_CONFIG     = "pac_config.json"
        pac_json       = os.path.join(os.getcwd(), "boards", BOARD, "sf", "projects", PROJECT, PAC_CONFIG)
        PWD            = os.path.join(os.getcwd())
        SDTOOLS        = os.path.join(os.getcwd(), "tools", "sdtools")
        BUILD_PATH     = os.path.join(os.getcwd(), "boards", BOARD, "build")
        PREBUILD_PATH  = os.path.join(os.getcwd(), "prebuilts")
        TEMP_PATH      = os.path.join(SDTOOLS, "temp")
        BIN_PATH       = os.path.join(SDTOOLS, "bin")
        PAC_TEMP_PATH  = os.path.join(SDTOOLS, "pac", BOARD, PROJECT)
        PAC_PTAH       = os.path.join(BUILD_PATH,  PROJECT,  TOOLCHAIN)
        dloader        = os.path.join(BUILD_PATH, "dloader", "gcc", "sf", "sf.bin")
        bootloader     = os.path.join(BUILD_PATH, "bootloader", TOOLCHAIN, "sf", "sf.bin")
        ssdk_bin_file  = os.path.join(BUILD_PATH,  PROJECT,  TOOLCHAIN, "*/*.bin")
        fat_file       = os.path.join(PREBUILD_PATH, "fat", "*")
        other_file     = os.path.join(PREBUILD_PATH, "other", "*")

        sysstr = platform.system()
        if sysstr =="Windows":
            PYTHON = "../../prebuilts/windows/python-3.7.0/python.exe "
            ATB_SIGNER = "./sign_tool/windows/atb_signer.exe"
            print ("Call Windows tasks")

        else:
            PYTHON = "python3 "
            ATB_SIGNER = "./sign_tool/linux/atb_signer"
            print ("Call Linux tasks")

        try:
            gen_pac(BOARD, PROJECT)
        except Exception as e:
             print("gen_pac error:\n    %s" % e)
             sys.exit(-1)
    except Exception as e:
        print("error:\n    %s" % e)
        sys.exit(-1)
