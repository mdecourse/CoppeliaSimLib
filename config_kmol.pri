# location of boost headers:
    BOOST_INCLUDEPATH = "y:/boost_1_76_0"    # (e.g. Windows)
    #BOOST_INCLUDEPATH = "/usr/local/include"    # (e.g. MacOS)
 
# location of lua headers:
    LUA_INCLUDEPATH = "y:\lua-5.4.3_src\src"    # (e.g. Windows)
    #LUA_INCLUDEPATH = "../../../../../mingw64/include/lua5.1"    # (e.g. Windows-MSYS2)
    #LUA_INCLUDEPATH = "../../lua5_1_4_Linux26g4_64_lib/include"    # (e.g. Ubuntu)
    #LUA_INCLUDEPATH = "/usr/local/include/lua5.1"    # (e.g. MacOS)
 
# lua libraries to link:
    LUA_LIBS = "y:\lua-5.4.3_src\src\liblua.a"    # (e.g. Windows)
    #LUA_LIBS = -llua5.1    # (e.g. Windows-MSYS2)
    #LUA_LIBS = -L"../../lua5_1_4_Linux26g4_64_lib/" -llua5.1    # (e.g. Ubuntu)
    #LUA_LIBS = "/usr/local/lib/liblua5.1.dylib"    # (e.g. MacOS)
 
# qscintilla location:
    QSCINTILLA_DIR = "y:/QScintilla_gpl-2.11.2"    # (e.g. Windows)
    #QSCINTILLA_DIR = "../../QScintilla-commercial-2.7.2"    # (e.g. Windows-MSYS2)
    #QSCINTILLA_DIR = "../../QScintilla-commercial-2.7.2"    # (e.g. Ubuntu)
    #QSCINTILLA_DIR = "../../QScintilla-commercial-2.7.2"    # (e.g. MacOS)
 
# qscintilla headers:
    QSCINTILLA_INCLUDEPATH = "$${QSCINTILLA_DIR}/include" "$${QSCINTILLA_DIR}/Qt4Qt5"
 
# qscintilla libraries to link:
    QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/libqscintilla2_qt5.dll.a"    # (e.g. Windows)
    #QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/release/release/libqscintilla2.dll.a"    # (e.g. Windows-MSYS2)    
    #QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/release/libqscintilla2.so"    # (e.g. Ubuntu)
    #QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/release/libqscintilla2.9.0.2.dylib"    # (e.g. MacOS)
# Make sure if a config.pri is found one level above, that it will be used instead of this one:
    exists(../config_kmol.pri) { include(../config_kmol.pri) }
