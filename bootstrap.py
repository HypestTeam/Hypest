#!/usr/bin/env python

import sen as senpai
import os, sys
import argparse

# command line stuff
parser = argparse.ArgumentParser(usage='%(prog)s [options...]')
parser.add_argument('--debug', action='store_true', help='compile with debug flags')
parser.add_argument('--cxx', metavar='<compiler>', help='compiler name to use (default: g++)', default='g++')
parser.add_argument('--quiet', '-q', action='store_true', help='suppress warning output')
args = parser.parse_args()

project = senpai.Project(name='Hypest', compiler=senpai.compiler(args.cxx), builddir='bin', objdir='obj')
project.includes = ['.', 'include']
project.dependencies = ['deps']
project.libraries = ['curl']
S = senpai.Executable(name='hypest', target='build', run='run')
S.files = senpai.files_from('src', '*.cpp')

def warning(string):
    if not args.quiet:
        print('warning: {}'.format(string))

# configuration
if 'g++' not in args.cxx:
    warning('compiler not explicitly supported: {}'.format(args.cxx))

cxxflags = ['-Wall', '-Wextra', '-pedantic', '-std=c++11', '-Wno-missing-field-initializers']

if sys.platform == 'win32':
    project.libraries.append('curldll')

if args.debug:
    cxxflags.extend(['-g', '-O0', '-DDEBUG'])
else:
    cxxflags.extend(['-DNDEBUG', '-O3'])

if args.cxx == 'clang++':
    cxxflags.extend(['-Wno-constexpr-not-const', '-Wno-unused-value', '-Wno-mismatched-tags'])

project.flags = cxxflags
project.add_executable(S)
project.dump(open('build.ninja', 'w'))
