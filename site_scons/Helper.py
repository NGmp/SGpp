import glob
import os
import sys
import subprocess
import shlex

# get all files in a folder matching "SConscript*"
# path has to end with "/"
def getModules():
    
#     if path[-1] != '/':
#         path += '/'
    path = '#/'
    suffix = '/SConscript'
    searchString = path + '*' + suffix
    modulePaths = glob.glob(searchString)
    modules = []
    for modulePath in modulePaths:
        module = modulePath[:-len(suffix)]
        module = module[len(path):]        
        modules.append(module)
    return modules

# Definition of flags / command line parameters for SCons
#########################################################################

def multiParamConverter(s):
    print s
    return s.split(',')

# detour compiler output
def print_cmd_line(s, target, src, env):
    if env['VERBOSE']:
        sys.stdout.write(u'%s\n' % s)
    else:
        sys.stdout.write(u'.')
        sys.stdout.flush()
    if env['CMD_LOGFILE']:
        open(env['CMD_LOGFILE'], 'a').write('%s\n' % s);