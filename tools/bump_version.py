#!/usr/bin/env python3

import re
import subprocess

VERSION_LINE_TEMPLATE = 'const std::string LIBKULLO_VERSION = "{}";\n'
VERSION_PATTERN = 'LIBKULLO_VERSION = "(v([0-9]+)-dev?)"'

def write_versioncpp(lines):
    with open("../kulloclient/util/version.cpp", "w") as f:
        f.write("".join(lines))

def read_versioncpp():
    lines = []
    with open("../kulloclient/util/version.cpp") as f:
        for line in f:
            lines.append(line)
    return lines

def main():
    version_full = None
    version_number = None
    versioncpp_version_line_number = -1
    versioncpp_lines = read_versioncpp()

    for index, line in enumerate(versioncpp_lines):
        match = re.search(VERSION_PATTERN, line)
        if match:
            versioncpp_version_line_number = index
            version_full = match.group(1)
            version_number = int(match.group(2))

    if not version_full:
        raise Exception("Version not found")

    new_version1 = "v{}".format(version_number)
    new_version2 = "v{}-dev".format(version_number+1)

    versioncpp_lines[versioncpp_version_line_number] = \
        VERSION_LINE_TEMPLATE.format(new_version1)
    write_versioncpp(versioncpp_lines)
    cmd = ["git", "add", "../kulloclient/util/version.cpp"]
    subprocess.check_call(cmd)
    cmd = ["git", "commit", "-mSet version: {}".format(new_version1)]
    subprocess.check_call(cmd)
    cmd = ["git", "tag", "--message=", "--annotate", new_version1]
    subprocess.check_call(cmd)

    versioncpp_lines[versioncpp_version_line_number] = \
        VERSION_LINE_TEMPLATE.format(new_version2)
    write_versioncpp(versioncpp_lines)
    cmd = ["git", "add", "../kulloclient/util/version.cpp"]
    subprocess.check_call(cmd)
    cmd = ["git", "commit", "-mSet version: {}".format(new_version2)]
    subprocess.check_call(cmd)

main()
