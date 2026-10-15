#!/usr/bin/python3
# -*- coding: UTF-8 -*-

class FileBase:

    STATEMENT = """
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
"""
    prefix = STATEMENT
    suffix = ""

    def __init__(self, filename):
        self.filename = filename
        self.file = open(self.filename, 'w+', newline="\n", encoding="utf-8")
        self.file.write(self.prefix)

    # executed on entering the 'with' statement
    def __enter__(self):
        return self

    # executed on exiting the 'with' statement
    def __exit__(self, exc_type, exc_value, traceback):
        if self.file:
            self.file.write(self.suffix)
            self.file.flush()
            self.file.close()

    def write(self, data):
        if self.file:
            self.file.write(data)
        else:
            raise Exception("Failed to write data.")

    def writelines(self, data):
        if self.file:
            self.file.writelines(data)
        else:
            raise Exception("Failed to writelines.")