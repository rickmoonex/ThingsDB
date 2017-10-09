#!/usr/bin/python3
import sys
import os
import datetime
import stat

CTEMPLATE = """/*
 * test_{name}.c
 *
 *  Created on: {today}
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include "test.h"
#include <{header}>



int main()
{{
    test_start("{name}");

    /* test adding values */
    {{
        assert (1 + 1 == 2);
    }}

    test_end(0);
    return 0;
}}
"""

SHTEMPLATE = """#!/bin/bash
NAME=test_{name}
C_SRC=\\
"{source} "\\

. test.sh
"""

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: new_test.py <source.c>")
        exit(1)

    source = sys.argv[1]

    if not os.path.isfile(source):
        print("cannot find source file:", source)
        exit(1)

    p, fn = os.path.split(source)
    name, _ = os.path.splitext(fn)

    header = ['{}.h'.format(name)]

    while 1:
        p, folder = os.path.split(p)
        if folder != "src":
            header.insert(0, folder)
        else:
            break

    header = os.path.join(*header)

    shfn = "test_{}.sh".format(name)
    if os.path.exists(shfn):
        print("file already exists:", shfn)
        exit(1)

    cfn = "test_{}.c".format(name)
    if os.path.exists(cfn):
        print("file already exists:", cfn)
        exit(1)

    today = "{:%b %d, %Y}".format(datetime.date.today())

    with open(shfn, "w") as f:
        f.write(SHTEMPLATE.format(name=name, source=source))

    with open(cfn, "w") as f:
        f.write(CTEMPLATE.format(name=name, today=today, header=header))

    st = os.stat(shfn)
    os.chmod(shfn, st.st_mode | stat.S_IEXEC)