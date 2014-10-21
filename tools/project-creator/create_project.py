#!/usr/bin/python
# create_project.py
# Create Supber Bile OpenGL project
# Author: Kook

# define global variables
context = {
    "src_project_name"  : "undefined",
    "dst_project_name"  : "undeifned",
    "src_project_path"  : "undefined",
    "dst_project_path"  : "undefined",
    "chapter_name"      : "undefined",
    "script_dir"        : "undefined",
}

# begin
import sys
import os, os.path
import json
import shutil
import string

def dumpUsage():
    print "Usage: "
    print "Sample 1: ./create_project.py -c MyChapter -p MyGame"
    print ""

def checkParams(context):
    # generate our internal params
    context["script_dir"] = os.getcwd() + "/"
    global platforms_list
    
    # invalid invoke, tell users how to input params
    if len(sys.argv) < 2:
        dumpUsage()
        sys.exit()
    
    # find our params
    for i in range(1, len(sys.argv)):
        if "-c" == sys.argv[i]:
            # read the next param as chapter_name
            context["chapter_name"] = sys.argv[i+1]
        elif "-p" == sys.argv[i]:
            # read the next param as project_name
            context["dst_project_name"] = sys.argv[i+1]

    if context["dst_project_name"].isdigit():
        context["dst_project_name"] = "%02d_%s" % (string.atoi(context["chapter_name"]), context["dst_project_name"])
    else:
        context["dst_project_name"] = context["dst_project_name"]

    # set project path
    context["dst_project_path"] = os.getcwd() + "/../../projects/" + context["dst_project_name"]
    
    # pinrt error log our required paramters are not ready
    raise_error = False
    if context["dst_project_name"] == "undefined":
        print "Invalid -p parameter"
        raise_error = True
    if context["chapter_name"] == "undefined":
        print "Invalid -c parameter"
        raise_error = True
    if raise_error != False:
        sys.exit()

    # fill in src_project_name and src_package_name according to "language"
    context["src_project_name"] = "HelloCpp"
    context["src_project_path"] = os.getcwd() + "/../../template/HelloCpp"

def replaceString(filepath, src_string, dst_string):
    content = ""
    f1 = open(filepath, "rb")
    for line in f1:
        #print(line)
        if src_string in line:
            content += line.replace(src_string, dst_string)
        else:
            content += line
    f1.close()
    f2 = open(filepath, "wb")
    f2.write(content)
    f2.close()

def processProjects():
    # determine proj_path
    proj_path = context["dst_project_path"]
    java_package_path = ""

    # read josn config file or the current platform
    f = open("win32.json")
    data = json.load(f)

    # rename files and folders
    for i in range(0, len(data["rename"])):
        tmp = data["rename"][i].replace("PACKAGE_PATH", java_package_path)
        src = tmp.replace("PROJECT_NAME", context["src_project_name"])
        dst = tmp.replace("PROJECT_NAME", context["dst_project_name"])
        if ( os.path.isfile(proj_path + '/' + src) ):
            os.rename(proj_path + '/' + src, proj_path + '/' + dst)

    # remove useless files and folders
    for i in range(0, len(data["remove"])):
        dst = data["remove"][i].replace("PROJECT_NAME", context["dst_project_name"])
        if (os.path.exists(proj_path + dst) == True):
            shutil.rmtree(proj_path + dst)
    
    # rename project_name
    for i in range(0, len(data["replace_project_name"])):
        dst = data["replace_project_name"][i].replace("PROJECT_NAME", context["dst_project_name"])
        if ( os.path.isfile(proj_path + '/' + dst) ):
            replaceString(proj_path + '/' + dst, context["src_project_name"], context["dst_project_name"])
            replaceString(proj_path + '/' + dst, "main.cpp", context["dst_project_name"] + ".cpp")

    # done!
    print "creat %s\t\t: Done!" % context["dst_project_name"]


# -------------- main --------------
if __name__ == "__main__":
    # dump argvs
    # print sys.argv
    
    # prepare valid "context" dictionary
    checkParams(context)
    
    # copy "lauguage"(cpp/lua/javascript) platform.proj into cocos2d-x/projects/<project_name>/folder
    if (os.path.exists(context["dst_project_path"]) == True):
        print "Error:" + context["dst_project_path"] + " folder is already existing"
        print "Please remove the old project or choose a new PROJECT_NAME in -project parameter"
        sys.exit()
    else:
        shutil.copytree(context["src_project_path"], context["dst_project_path"], True)
    
    # call process_proj from each platform's script folder          
    processProjects()
    
    #print "New project has been created in this path: " + context["dst_project_path"].replace("/tools/project-creator/../..", "")
    print "Have Fun!"

