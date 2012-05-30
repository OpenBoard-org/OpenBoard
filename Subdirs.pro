TEMPLATE = subdirs
CONFIG += recursive
CONFIG += debug_and_release

win32:        SUB_DIR = win32
macx:         SUB_DIR = macx
linux-g++:    SUB_DIR = linux
linux-g++-32: SUB_DIR = linux
linux-g++-64: SUB_DIR = linux

BUILD_DIR = build/$$SUB_DIR

CONFIG(debug, debug|release):BUILD_DIR = $$BUILD_DIR/debug
CONFIG(release, debug|release) {
   BUILD_DIR = $$BUILD_DIR/release
   CONFIG += warn_off
}

SANKORE_DIR = $$PWD
SANKORE_PRO = $$SANKORE_DIR/Sankore_3.1.pro 
THIRD_PARTY_DIR = $$PWD/Sankore-ThirdParty
SANKORE_PLUGINS_DIR = $$SANKORE_DIR/plugins
PLUGIN_CFF_ADAPTOR_DIR = $$SANKORE_PLUGINS_DIR/cffadaptor
PLUGIN_CFF_ADAPTOR_PRO = $$PLUGIN_CFF_ADAPTOR_DIR/UBCFFAdaptor.pro

SUBDIRS += $$SANKORE_PRO

if(exists($$PLUGIN_CFF_ADAPTOR_DIR)){
    SUBDIRS += $$PLUGIN_CFF_ADAPTOR_PRO
    $$SANKORE_PRO.depends = $$PLUGIN_CFF_ADAPTOR_PRO
}

recurse_target = $$SANKORE_PRO

