# -*- coding: utf-8 -*-
# Copyright@semidrive, May,12,2022

import sys
import os
import getopt
import traceback
import platform
import shutil
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element, SubElement, ElementTree


def _serialize_xml(write, elem, encoding, qnames, namespaces):
    tag = elem.tag
    text = elem.text
    if tag is ET.Comment:
        write("<!--%s-->" % ET._encode(text, encoding))
    elif tag is ET.ProcessingInstruction:
        write("<?%s?>" % ET._encode(text, encoding))
    else:
        tag = qnames[tag]
        if tag is None:
            if text:
                write(str.encode(ET._escape_cdata(text, encoding)))
            for e in elem:
                _serialize_xml(write, e, encoding, qnames, None)
        else:
            bLine = str.encode("<" + tag)
            write(bLine)
            items = elem.items()
            if items or namespaces:
                if namespaces:
                    for v, k in sorted(namespaces.items(),
                            key=lambda x: x[1]):  # sort on prefix
                        if k:
                            k = ":" + k
                        write(" xmlns%s=\"%s\"" % (
                            k.encode(encoding),
                            v
                        ))
                # for k, v in sorted(items):  # lexical order
                for k, v in items:  # Monkey patch
                    if isinstance(k, ET.QName):
                        k = k.text
                    if isinstance(v, ET.QName):
                        v = qnames[v.text]
                    # else:
                    #     v = ET._escape_attrib(v)
                    v = v.replace('\"', '&quot;')
                    write(str.encode(" %s=\"%s\"" % (qnames[k], v)))
            if text or len(elem):
                write(b'>')
                if text:
                    write(str.encode(ET._escape_cdata(text)))
                for e in elem:
                    _serialize_xml(write, e, encoding, qnames, None)
                write(str.encode("</" + tag + ">"))
            else:
                write(str.encode("/>"))
    if elem.tail:
        write(str.encode(ET._escape_cdata(elem.tail)))

ET._serialize_xml = _serialize_xml

def _serialize_text(write, elem, encoding):
    for part in elem.itertext():
        write(part.encode(encoding))
    if elem.tail:
        write(elem.tail.encode(encoding))

ET._serialize_text = _serialize_text
_serialize = {
    "prj": _serialize_xml,
    "xml": _serialize_xml,
    "html": ET._serialize_html,
    "text": _serialize_text,
    # this optional method is imported at the end of the module
    # "c14n": _serialize_c14n,
}
ET._serialize = _serialize

def write(self, file_or_filename, encoding="UTF-8", xml_declaration=True, default_namespace=None, method=None):
    # assert self._root is not None
    if not method:
        method = "xml"
    elif method not in _serialize:
        # FIXME: raise an ImportError for c14n if ElementC14N is missing?
        raise ValueError("unknown method %r" % method)
    if hasattr(file_or_filename, "write"):
        file = file_or_filename
    else:
        file = open(file_or_filename, "wb")
        # file = codecs.open(file_or_filename,"w","utf-8-sig")
    write = file.write
    if not encoding:
        if method == "c14n":
            encoding = "utf-8"
        else:
            encoding = "us-ascii"
    elif xml_declaration or (xml_declaration is None and
                                    encoding not in ("utf-8", "us-ascii")):
        if method == "xml":
            bLine = str.encode('<?xml version="1.0" encoding="%s" standalone="no"?>\n<?fileVersion 4.0.0?>' % encoding)
            write(bLine)
            pass
        elif method == "prj":
            bLine = str.encode('<?xml version="1.0" encoding="%s"?>\n' % encoding)
            write(bLine)
            pass
    if method == "text":
        _serialize_text(write, self._root, encoding)
    else:
        qnames, namespaces = ET._namespaces(
            self._root, default_namespace
        )
        serialize = _serialize[method]
        serialize(write, self._root, encoding, qnames, namespaces)
    if file_or_filename is not file:
        file.close()

ET.ElementTree.write = write

def pretty_xml(element, indent, newline, level=0):  # elemnt为传进来的Elment类，参数indent用于缩进，newline用于换行
    if element:  # 判断element是否有子元素
        if (element.text is None) or element.text.isspace():  # 如果element的text没有内容
            element.text = newline + indent * (level + 1)
        else:
            element.text = newline + indent * (level + 1) + element.text.strip() + newline + indent * (level + 1)
            # else:  # 此处两行如果把注释去掉，Element的text也会另起一行
            # element.text = newline + indent * (level + 1) + element.text.strip() + newline + indent * level
    temp = list(element)  # 将element转成list
    for subelement in temp:
        if temp.index(subelement) < (len(temp) - 1):  # 如果不是list的最后一个元素，说明下一个行是同级别元素的起始，缩进应一致
            subelement.tail = newline + indent * (level + 1)
        else:  # 如果是list的最后一个元素， 说明下一行是母元素的结束，缩进应该少一个
            subelement.tail = newline + indent * level
        pretty_xml(subelement, indent, newline, level=level + 1)  # 对子元素进行递归操作

def show_usage():
    print("gen_ssdk_rapidstudio_prj version: 1.22.1401")
    print("usage: gen_ssdk_rapidstudio_prj.py <--root sdk_root> <--board board> <--template template_dir> [-h]")
    print("options:")
    print("--rootdir    sdk_root_dir    Specify ssdk root directory")
    print("--board      board_name      Specify board name")
    print("--project    project_name    Specify project name")
    print("--core       core_name       Specify core name")
    print("--template   template_dir    Specify IDE project template file directory")
    print("--GLOBAL_OPTFLAGS        OPTFLAGS    Specify optimization parameter")
    print("--GLOBAL_COMPILEFLAGS    COMPILEFLAGS Specify global compile parameter")
    print("--GLOBAL_CFLAGS          CFLAGS      Specify c parameter")
    print("--GLOBAL_ASMFLAGS        ASMFLAGS    Specify asm parameter")
    print("--SOURCES                SOURCES     Specify sources parameter")
    print("--INCLUDES               INCLUDES    Specify inclues parameter")
    print("--LINKLDS                LINKLDS     Specify link parameter")
    print("--LINKLIBS               LINKLIBS    Specify link lib parameter")
    print("-h                           help")
    print("e.g.: gen_ssdk_rapidstudio_prj.py --root ./ssdk --board g9x_safety_ref --template ./ide_prj_template")

def copy_output_files(**_kv):
    _prj_name = 'build-' + _kv['board'].split('/')[-1]

    _cfg_path = _kv['root'] + '/out/' + _prj_name + '/config.h'
    # shutil.copyfile(_cfg_path, _kv['board'] + '/config.h')

    _prj_out_path = _kv['root'] + '/out/' + _prj_name
    names = os.listdir(_prj_out_path)
    for name in names:
        _src_path = _prj_out_path + '/' + name
        # print(_src_path)
        if os.path.isfile(_src_path):
            _file_name, _ext_name = os.path.splitext(_src_path)
            if _ext_name.lower() in ['.ld']:
                _dst_path = _kv['board'] + '/' + name
                shutil.copyfile(_src_path, _dst_path)
                print("copy", _src_path, "to", _dst_path)


def extract_params_by_cmd(**_kv):

    src_list = []
    c_define_list = []
    s_define_list = []
    c_include_list = []
    s_include_list = []

    w_define_list = []
    f_define_list = []
    u_define_list = []

    link_lib_list = []
    link_dir_list = []

    _dict_marco = {
        '-W': w_define_list,
        '-f': f_define_list,
        '-U': u_define_list,
    }

    _file_info = {}

    config_h = ''
    top_path = ""
    entry_flag = False
    entry_opt = ''
    entry_src = ''
    _para = {}
    key_list = list(_kv.keys())
    for _k in key_list:
    #for _k, _v in _kv.items():
        _v = _kv[_k]
        # print(_k, ":", _v)
        
        if _k == "rootdir":
            top_path = _v.replace('\\', '/')
            top_path += '/'
            print("top_dir : ", top_path)
        elif _k == "GLOBAL_COMPILEFLAGS":
            # --GLOBAL_COMPILEFLAGS "-g -finline -include boards/g9x_safety_ref/config.h -Werror -Wall -Wsign-compare -fno-short-enums -Wno-multichar 
            # -Wno-unused-function -Wno-unused-label -Wno-tautological-compare -Wno-discarded-qualifiers -Wno-sign-compare -fno-common -Wno-nonnull-compare 
            # -U__INT32_TYPE__ -U__UINT32_TYPE__  -mcpu=cortex-r5 -mfpu=vfpv3-d16 -mfloat-abi=hard -mthumb -D__thumb__"

            tmp_list = _v.split(" ")
            if len(tmp_list):
                _index = -1
                for item in tmp_list:
                    _index += 1
                    if len(item) < 2:
                        continue
                    if item == '-include':
                        config_h = tmp_list[_index]
                        continue
                    if item in ["-Werror"]:
                        continue
                    _prefix = item[0:2]
                    if _prefix in ['-W', '-f', '-U']:
                        _list = _dict_marco.get(_prefix, None)
                        if _list is not None and item[2:] not in _list:
                            _list.append(item[2:])
                    elif _prefix == '-D':
                        c_define_list.append(item[2:])
                        s_define_list.append(item[2:])
        
        elif  _k == 'GLOBAL_OPTFLAGS':
            # --GLOBAL_OPTFLAGS "-O2"
            _para['g_opt_flag'] = _v[_v.find('-'):]
        
        elif _k in ['GLOBAL_CFLAGS', 'GLOBAL_ASMFLAGS']:
            # --GLOBAL_CFLAGS "--std=c11 -Wwrite-strings -DCONFIG_NEWLIB"
            # --GLOBAL_ASMFLAGS "-DASSEMBLY"
            define_list = s_define_list
            if  _k == 'GLOBAL_CFLAGS':
                define_list = c_define_list
            tmp_list = _v.split(" ")
            if len(tmp_list):
                for item in tmp_list:
                    if len(item) < 2:
                        continue
                _prefix = item[0:2]
                if _prefix == '-D':
                    if item[2:] not in define_list:
                        define_list.append(item[2:])
                elif _prefix in ['-W', '-f', '-U']:
                    _list = _dict_marco.get(_prefix, None)
                    if _list is not None and item[2:] not in _list:
                        _list.append(item[2:])
                # elif _prefix == '-W':
                #         _file_info[entry_src]['warns'] += item.strip() + ' '
                #     elif _prefix == '-f':
                #         _file_info[entry_src]['opts'] += item.strip() + ' '
        elif _k == 'SOURCES':
            # --SOURCES " boards/g9x_safety_ref/board.c boards/g9x_safety_ref/clock_cfg.c"
            tmp_list = _v.split(" ")
            if len(tmp_list):
                for item in tmp_list:
                    if len(item) == 0:
                        continue
                    _src = item.replace('\\', '/').replace('//', '/').replace(top_path, '').rstrip()
                    src_list.append(_src)

        elif _k == 'INCLUDES':
            # --INCLUDES "-I./out/build-g9x_safety_ref -I./include -Iboards/g9x_safety_ref"
            tmp_list = _v.split(" ")
            if len(tmp_list):
                for item in tmp_list:
                    if len(item) < 2:
                        continue
                    _prefix = item[0:2]
                    if _prefix == '-I':
                        if item[2:] not in s_include_list:
                            s_include_list.append(item[2:])
                        if item[2:] not in c_include_list:
                            c_include_list.append(item[2:])
                    else:
                        if item not in s_include_list:
                            s_include_list.append(item)
                        if item not in c_include_list:
                            c_include_list.append(item)
        elif _k == 'LINKLIBS':
            # --LINKLIBS "-Ldrivers/taishan/asw/libs -lasw -Ldrivers/taishan/dispss/libs -ldispss -Ldrivers/taishan/g2dlite/libs -lg2dlite"
            # print("LINKLIBS", _v)
            tmp_list = _v.split(" ")
            if len(tmp_list):
                for item in tmp_list:
                    _prefix = item[0:2]
                    if _prefix in ['-l']:
                        if item[2:] not in link_lib_list:
                            link_lib_list.append(item[2:])
                    elif _prefix in ['-L']:
                        if item[2:] not in link_dir_list:
                            link_dir_list.append(item[2:])

    _para['c_include'] = c_include_list
    _para['s_include'] = s_include_list
    _para['link_src'] = src_list

    _para['c_define'] = c_define_list
    _para['s_define'] = s_define_list

    _para['g_warns'] = w_define_list
    _para['g_opts'] = f_define_list
    _para['g_undef'] = u_define_list
    _para['config_h'] = config_h

    _para['link_lib'] = link_lib_list
    _para['link_dir_list'] = link_dir_list

    _para['file_info'] = _file_info
    return _para

def gen_project_files(**_kv):
    # \ssdk\boards\e3_evb\build\ref\rapidstudio\sf
    ide_path = _kv['rootdir'] + '/boards/' + _kv['board'] + '/build/' + _kv['project'] +'/rapidstudio/' + _kv['core']
   
    os.makedirs(ide_path, exist_ok=True)
    _folders = ide_path.split('/')
    project_name = _kv['board'] + '_' + _kv['project'] + '_' + _kv['core']
    depth = len(_folders) - len(_kv['rootdir'].split('/'))

    _src_prj = _kv['template'] + '/.project'
    _src_c_prj = _kv['template'] + '/.cproject'

    for _path in [_src_prj, _src_c_prj]:
        if not os.path.exists(_path):
            raise Exception("Not found: ", _path)

    _dst_prj = ide_path + '/.project'
    _dst_c_prj = ide_path + '/.cproject'

    # gen .project file
    tree = ET.parse(_src_prj)
    root = tree.getroot()
    
    print("generated rapiadstudio project to path:", ide_path)
    print("project_name:", project_name)
    list(root)[0].text = _kv['core']  # project_name

    sdk_root_path = _kv['rootdir']

    _link_res = SubElement(root, 'linkedResources', {})
    for _src_path in _kv['link_src']:
        _path = sdk_root_path + '/' + _src_path
        # print("_path", _path)
        if _path.startswith(ide_path):
            # print("skip", _path)
            continue

        _link = SubElement(_link_res, 'link', {})
        SubElement(_link, 'name', {}).text = 'ssdk' + '/' + _src_path
        SubElement(_link, 'type', {}).text = '1'
        SubElement(_link, 'locationURI', {}).text = '$%7BPARENT-{0}-PROJECT_LOC%7D'.format(depth) + '/' + _src_path

    pretty_xml(root, '\t', '\n')
    tree.write(_dst_prj, method="prj")

    # gen .cproject file
    src_env_dir = '${ProjDirPath}/'
    for i in range(depth):
        src_env_dir += '../'
    src_env_dir = src_env_dir[:-1]
    # print("src_env_dir", src_env_dir)

    PRJ_DIR_ENV = '&quot;${SSDK_ROOT_PATH}'  # '&quot;${' + project_name.upper() + '_ENV_PATH' + '}'
    PRJ_DIR_VAR =  'SSDK_ROOT_PATH'  # project_name.upper() + '_ENV_PATH'
    #SRC_LNK_VAR = project_name.upper() + '_LNK_PATH'

    Assembler_tag = "GNU Arm Cross Assembler".lower()
    CC_tag = "GNU Arm Cross C Compiler".lower()
    Linker_tag = "GNU ARM Cross C Linker".lower()

    SYMBOLS_KEY = "Defined symbols (-D)".lower()
    USER_INCLUDE_KEY = "Include paths (-I)".lower()
    SYS_INCLUDE_KEY = "Include system paths (-isystem)".lower()
    LINK_KEY = "Script files (-T)".lower()

    OPT_LEVEL = "Optimization Level".lower()
    OTHER_TARGET = "Other target flags".lower()
    OTHER_OPT = "Other optimization flags".lower()
    OTHER_WARN = "Other warning flags".lower()
    OTHER_LINKER = "Other linker flags".lower()

    LIB_LINKER_DIR = "Library search path (-L)".lower()
    LIB_LINKER_FILE = "Libraries (-l)".lower()

    tree = ET.parse(_src_c_prj)
    root = tree.getroot()

    cfg_elf_list = []
    c_compiler_list = []

    # retrieve cconfiguration id and compiler id
    ccfg_module = root.find("storageModule")
    for ccfg in ccfg_module:
        cfg_elf_list.append(ccfg.attrib['id'])

    for tool in root.findall("storageModule/cconfiguration/storageModule/configuration/folderInfo/toolChain/"):
        if "name" not in tool.attrib.keys():
            continue
        if tool.attrib['name'].lower() in [Assembler_tag, CC_tag]:
            _tool_name = tool.attrib['name'].lower()
            # print("\ndeal with : ", tool.attrib['name'])

            if _tool_name == CC_tag:
                c_compiler_list.append(tool.attrib['id'])

            for option_item in list(tool):
                if "name" not in option_item.attrib.keys():
                    continue
                _attr_name = option_item.attrib['name'].lower()
                if _attr_name == SYMBOLS_KEY:
                    _list_def = None
                    if CC_tag == _tool_name:
                        _list_def = _kv['c_define']
                    else:
                        _list_def = _kv['s_define']
                    for symbol in _list_def:
                        # print("  defined : ", symbol)
                        attrib = {
                            "builtIn": "false",
                            'value': symbol
                        }
                        SubElement(option_item, 'listOptionValue', attrib)
                elif _attr_name in [USER_INCLUDE_KEY]: #, SYS_INCLUDE_KEY]:
                    _list_include = None
                    if CC_tag == _tool_name:
                        _list_include = _kv['c_include']
                    else:
                        _list_include = _kv['s_include']
                    for include_path in _list_include:
                        attrib = {
                            "builtIn": "false",
                            'value': PRJ_DIR_ENV + '/' + include_path + '&quot;'
                        }
                        SubElement(option_item, 'listOptionValue', attrib)

        elif tool.attrib['name'].lower() in [OTHER_OPT, OTHER_WARN]:
            _attr_name = tool.attrib['name'].lower()
            if _attr_name == OTHER_OPT:
                _val = ''
                for opt in _kv['g_opts']:
                    if not opt.startswith('-f'):
                        opt = '-f' + opt
                    _val += opt + ' '
                tool.attrib['value'] = _val.strip()
            elif _attr_name == OTHER_WARN:
                _val = ''
                for opt in _kv['g_warns']:
                    if not opt.startswith('-W'):
                        opt = '-W' + opt
                    _val += opt + ' '
                _val += ' -Wno-format'  # append for ssdk
                tool.attrib['value'] = _val.strip()

        elif tool.attrib['name'].lower() == Linker_tag:
            for option_item in list(tool):
                if "name" not in option_item.attrib.keys():
                    continue
                # if option_item.attrib['name'].lower() == LINK_KEY:
                #     # print(option_item.attrib['name'])
                #     link_path = '&quot;${ProjDirPath}/link.ld&quot;'
                #     print("   linked : ", link_path)
                #     attrib = {
                #         "builtIn": "false",
                #         'value': link_path
                #     }
                #     SubElement(option_item, 'listOptionValue', attrib)

                if option_item.attrib['name'].lower() == OTHER_LINKER:
                    _link = _kv['LINKLDS'].replace('XXXX', '${SSDK_ROOT_PATH}/')
                    _link += " " + option_item.attrib['value']
                    option_item.attrib['value'] = '%s' % (_link)
                
                elif option_item.attrib['name'].lower() == LIB_LINKER_DIR:
                    if len(_kv['link_dir_list']):
                        for item in _kv['link_dir_list']:
                            link_path = '&quot;${SSDK_ROOT_PATH}/%s&quot;' % (item)
                            attrib = {
                                "builtIn": "false",
                                'value': link_path
                            }
                            SubElement(option_item, 'listOptionValue', attrib)
                elif option_item.attrib['name'].lower() == LIB_LINKER_FILE:
                    if len(_kv['link_lib']):
                        for item in _kv['link_lib']:
                            attrib = {
                                "builtIn": "false",
                                'value': item
                            }
                            SubElement(option_item, 'listOptionValue', attrib)
    
        elif tool.attrib['name'].lower() == OTHER_TARGET:
            # src: boards/e3_evb/sf/projects/ref/config.h
            # dst: boards\e3_evb\build\ref\rapidstudio\sf
            tool.attrib['value'] = '-include &quot;${ProjDirPath}/../../../../%s/projects/%s/config.h&quot;' % (_kv['core'], _kv['project'])
        elif tool.attrib['name'].lower() == OPT_LEVEL:
            tool.attrib['value'] = 'ilg.gnuarmeclipse.managedbuild.cross.option.optimization.level.' + {
                '-O0': 'none',
                '-O1': 'optimize',
                '-O2': 'more',
                '-O3': 'most',
                '-Os': 'size',
                '-Ofast': 'fast',
                '-Og': 'debug'
            }.get('-O0' if 'g_opt_flag' not in _kv.keys() else _kv['g_opt_flag'], 'none')

    # add for file info which its opt and warn flags differ from the others
    if len(_kv['file_info']) > 0:

        _hash_id = 1
        _index = 0

        for _cfg in root.findall("storageModule/cconfiguration/storageModule/configuration"):

            cfg_elf_id = cfg_elf_list[_index]
            c_compiler_id = c_compiler_list[_index]

            for _k, _v in _kv['file_info'].items():

                _name = _k.split('/')[-1].strip()
                _res_path = ''

                _path = sdk_root_path + '/' + _k
                if not _path.startswith(ide_path):
                    _res_path += 'ssdk/' + _k
                else:
                    _res_path = _name

                _attrib = {
                    'id': cfg_elf_id + '.' + str(_hash_id),
                    'name': _name,
                    'rcbsApplicability': 'disable',
                    'resourcePath': _res_path,
                    'toolsToInvoke': c_compiler_id + '.' + str(_hash_id)
                }
                _file_info = SubElement(_cfg, 'fileInfo', _attrib)

                _attrib_t1 = {
                    'id': c_compiler_id + '.' + str(_hash_id),
                    'name': 'GNU Arm Cross C Compiler',
                    'superClass': c_compiler_id
                }
                _tool1 = SubElement(_file_info, 'tool', _attrib_t1)

                _attrib_t2 = {
                    'id': 'org.eclipse.cdt.managedbuilder.ui.rcbs.' + str(_hash_id),
                    'name': 'Resource Custom Build Step',
                    'customBuildStep': 'true'
                }
                _tool2 = SubElement(_file_info, 'tool', _attrib_t2)

                SubElement(_tool1, 'option', {
                    'id': 'ilg.gnuarmeclipse.managedbuild.cross.option.c.compiler.otheroptimizations.' + str(_hash_id),
                    'name': 'Other optimization flags',
                    'superClass': 'ilg.gnuarmeclipse.managedbuild.cross.option.c.compiler.otheroptimizations',
                    'useByScannerDiscovery': 'true',
                    'value': _v['opts'].strip(),
                    'valueType': 'string'
                })
                SubElement(_tool1, 'option', {
                    'id': 'ilg.gnuarmeclipse.managedbuild.cross.option.c.compiler.otherwarnings.' + str(_hash_id),
                    'name': 'Other warning flags',
                    'superClass': 'ilg.gnuarmeclipse.managedbuild.cross.option.c.compiler.otherwarnings',
                    'useByScannerDiscovery': 'true',
                    'value': _v['warns'].strip(),
                    'valueType': 'string'
                })
                SubElement(_tool1, 'inputType', {
                    'id': 'ilg.gnuarmeclipse.managedbuild.cross.tool.c.compiler.input.' + str(_hash_id),
                    'superClass': 'ilg.gnuarmeclipse.managedbuild.cross.tool.c.compiler.input'
                })

                SubElement(SubElement(_tool2, 'inputType', 
                    {
                        'id': 'org.eclipse.cdt.managedbuilder.ui.rcbs.inputtype.' + str(_hash_id),
                        'name': 'Resource Custom Build Step Input Type'
                    }), 'additionalInput', {'kind': 'additionalinputdependency', 'paths': ''})

                SubElement(_tool2, 'outputType', {
                    'id': 'org.eclipse.cdt.managedbuilder.ui.rcbs.outputtype.' + str(_hash_id), 
                    'name': 'Resource Custom Build Step Output Type'})
                
                _hash_id += 1
            _index += 1

    # add build variable (for include macro, only take effective in IDE)
    for _node in root.findall("storageModule/cconfiguration/"):
        if 'buildSystemId' in _node.attrib.keys():
            _node_macros = SubElement(_node, 'macros', {})
            _attrib = {
                "name": PRJ_DIR_VAR,
                'type': 'VALUE_TEXT',
                'value': src_env_dir
            }
            SubElement(_node_macros, 'stringMacro', _attrib)

            _attrib = {
                "name": 'BOARD',
                'type': 'VALUE_TEXT',
                'value': _kv['board']
            }
            SubElement(_node_macros, 'stringMacro', _attrib)

            _attrib = {
                "name": 'PROJECT',
                'type': 'VALUE_TEXT',
                'value': _kv['project']
            }
            SubElement(_node_macros, 'stringMacro', _attrib)

            _attrib = {
                "name": 'CORE',
                'type': 'VALUE_TEXT',
                'value': _kv['core']
            }
            SubElement(_node_macros, 'stringMacro', _attrib)

    pretty_xml(root, '\t', '\n')
    print("generated project：", _dst_c_prj)
    tree.write(_dst_c_prj)

def gen_build_batch(**_kv):
    _prj_cfg_list = ['IRAM', 'DDR', 'Flash']
    _folders = _kv['board'].split('/')
    project_name = _folders[-1]

    for _prj_cfg in _prj_cfg_list:
        _batch = _kv['board'] + '/build_{}.bat'.format(_prj_cfg)
        if not os.path.exists(_batch):
            with open(_batch,'w') as fl:
                fl.writelines(['SDRapidStudio.bat -build {0} %CD% {1}\n'.format(project_name, _prj_cfg)])
                fl.close()

    _batch_main = _kv['board'] + '/build.bat'
    if not os.path.exists(_batch_main):
        with open(_batch_main,'w') as fl:
            fl.writelines(['start /WAIT build_{}.bat\n'.format(_prj_cfg) for _prj_cfg in _prj_cfg_list])
            fl.close()

def gen_jflash_batch(**_kv):
    # add for JLink JFlash
    _batch_dir = _kv['board'] + '/jlink/jflash'
    if not os.path.exists(_batch_dir):
        os.makedirs(_batch_dir)

        _src_prj = _kv['template'] + '/default.jflash'
        _dst_prj = _batch_dir + '/default.jflash'
        shutil.copyfile(_src_prj, _dst_prj)

        _batch = _batch_dir + '/jlink_flash.bat'
        with open(_batch,'w') as fl:
            fl.writelines([
                '@ECHO OFF\n',
                'if "%~1" EQU "" exit\n'
                'if "%2" EQU "" exit\n',
                'if "%3" EQU "" exit\n',
                'if "%4" EQU "" exit\n',
                '"%~1\JFlash.exe" -openprjdefault.jflash -open..\..\%3\%2.bin,%4 -auto -exit\n',
                'IF ERRORLEVEL 1 goto ERROR\n',
                'goto END\n',
                ':ERROR\n',
                'ECHO J-Flash ARM: Error!\n',
                ':END\n'
            ])
            fl.close()

    # add for JLink Commander
    _batch_dir = _kv['board'] + '/jlink/commander'
    if not os.path.exists(_batch_dir):
        os.makedirs(_batch_dir)

        _batch = _batch_dir + '/flash_download.bat'
        with open(_batch,'w') as fl:

            _bin_name = _kv['board'].split('/')[-1].strip()
            _cfg_list = ['Debug_IRAM', 'Debug_DDR', 'Debug_Flash',
                         'Release_IRAM', 'Release_DDR', 'Release_Flash']

            for _cfg in _cfg_list:
                _cmd_file = _batch_dir + '/cmd_flash_dl_' + _cfg + '.jlink'
                with open(_cmd_file,'w') as _fl:
                    _fl.writelines([
                        'si 1\n',
                        'speed 4000\n',
                        'r\n',
                        'h\n',
                        'loadbin ' + '..\\..\\' + _cfg + '\\' + _bin_name + '.bin,0x0\n'
                    ])
                    _fl.close()

            fl.writelines([
                '@ECHO OFF\n',
                'if "%~1" EQU "" exit\n'
                'if "%2" EQU "" exit\n',
                'if "%3" EQU "" exit\n',
                '"%~1\JLink.exe" -device %2 -ExitOnError -CommandFile ".\cmd_flash_dl_%3.jlink"\n'
            ])
            fl.close()

def search_actual_exec(dir_list, file):
    _exe_path = file
    for _dir in dir_list:
        _result = search(_dir, file)
        if _result is not None:
            _exe_path = _result
            break
    return _exe_path

def search(root, target):
    if not os.path.exists(root):
        return None
    os.chdir(root)
    items = os.listdir(os.curdir)
    for item in items:
        path = os.path.join(root, item)
        if os.path.isdir(path):
            _target_path = search(path, target)
            if _target_path is not None:
                return _target_path
            else:
                os.chdir(os.pardir)
        elif path.split('\\')[-1] == target:
            return path
    return None

def main(argv):
    """
    主函数
    :param argv: <--root sdk_root> <--board board> <--template template_dir> [-h]
    :return: 0:pass; other:fail
    """
    exit_code = 1
    ".cproject"
    ".project"

    try:
        _param = {
            'rootdir': '',
            'board': '',
            'project': '',
            'core': '',
            'template':'',
            "GLOBAL_OPTFLAGS": '',
            "GLOBAL_COMPILEFLAGS": '',
            "GLOBAL_CFLAGS": "",
            "GLOBAL_ASMFLAGS": "",
            "SOURCES": "",
            "INCLUDES": "",
            "LINKLDS": "",
            "LINKLIBS": ""
        }

        # accquire script parameters
        opts, args = getopt.getopt(
            argv, 
            "-h", 
            ["help", 
            'rootdir=',
            'board=',
            'project=',
            'core=',
            'template=',
            "GLOBAL_OPTFLAGS=",
            "GLOBAL_COMPILEFLAGS=",
            "GLOBAL_CFLAGS=",
            "GLOBAL_ASMFLAGS=",
            "SOURCES=",
            "INCLUDES=",
            "LINKLDS=",
            "LINKLIBS="
            ]
        )
        for opt, arg in opts:
            # print("opt", opt)
            # print("arg", arg)
            if opt == '-h':
                show_usage()
                sys.exit(0)
            else:
                for _key in _param.keys():
                    if opt.find(_key) != -1:
                        if _key in ['rootdir', 'template']:
                            arg = os.path.abspath(arg).replace('\\', '/')
                            if arg.endswith('/'):
                                arg = arg[:-1]
                        _param[_key] = arg
                        break

        # validations for basic params
        for _k, _v in _param.items():
            if len(_v) == 0 and _k not in ["LINKLIBS"]:
                show_usage()
                raise Exception('{} info is empty.'.format(_k))

        # copy related output files to project directory
        # copy_output_files(**_param)

        # extract params for .cproject (include paths, macros, link info, etc) 
        #print(_param)
        _kv = extract_params_by_cmd(**_param)
        _kv.update(_param)
        #print(_kv)

        # generate IDE project files by extracted params
        gen_project_files(**_kv)

        # generate auto build batch (build.bat)
        # gen_build_batch(**_param)

        # generate jlink j-flash project & batch file
        # gen_jflash_batch(**_param)

    except Exception:
        print("[Error] %s" % traceback.format_exc())
    else:
        print("Successfully generated rapidstudio project file!")
        exit_code = 0
    return exit_code


if __name__ == '__main__':
    main(sys.argv[1:])
    # main(['--root', 'D:\\project_sdk\\ssdk', 
    #       '--board', 'g9x_safety_ref', 
    #       '--template', 'D:\\project_sdk\\ide_prj_template'])
