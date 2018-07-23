QMAKE_CXXFLAGS += -std=c++0x

systemc_home = $$(SYSTEMC_HOME)
isEmpty(systemc_home) {
    systemc_home = /opt/systemc
}

systemc_target_arch = $$(SYSTEMC_TARGET_ARCH)
isEmpty(systemc_target_arch) {
}

unix:!macx {
    systemc_target_arch = linux64
    QMAKE_CXXFLAGS += -std=c++11 -O0 -g
}

macx: {
    systemc_target_arch = macosx64
    QMAKE_CXXFLAGS += -std=c++0x -stdlib=libc++ -O0 -g
}

INCLUDEPATH += $${systemc_home}/include
