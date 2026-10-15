#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import argparse
import sys
from project.project import Project

sys.path.append(os.path.dirname(__file__))

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--project", type=str, default="x9h_safety_ref")
    args = parser.parse_args()

    os.environ['SSDK_ROOT'] = '../../../'

    p = Project(args.project)

    p.prepare_project_gen()
    p.gen_modules_config()
    p.finish_project_gen()

