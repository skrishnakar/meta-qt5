require qt5.inc

SRC_URI += " \
    file://0001-Add-linux-oe-g-platform.patch \
    file://0002-qlibraryinfo-allow-to-set-qt.conf-from-the-outside-u.patch \
    file://0003-Add-external-hostbindir-option.patch \
    file://0004-qt_functions-temporary-remove-isEmpty-check.patch \
    file://0005-qmake-is-already-built-in-qtbase-native.patch \
    file://0006-Allow-building-a-separate-qmake-for-the-target.patch \
    file://0007-configureapp-Prefix-default-LIBDIRS-and-INCDIRS-with.patch \
    file://0008-qt_module-Fix-pkgconfig-replacement.patch \
    file://0009-qt_module-Fix-paths-in-.prl-files.patch \
    file://0010-wayland-scanner-disable-silent-rules.patch \
    file://0011-configure-don-t-export-SYSTEM_VARIABLES-to-.qmake.va.patch \
    file://0012-configure.prf-Allow-to-add-extra-arguments-to-make.patch \
    file://0013-Disable-mkv8snapshot.patch \
    file://0014-enables-tslib-device-to-be-read-from-env-variable.patch \
    file://0015-qtbase-allow-build-of-examples.patch \
    file://0018-QOpenGLPaintDevice-sub-area-support.patch \
    file://0019-Fix-FBO-restoring-in-QOpenGLTextureGlyphCache.patch \
    file://0020-Use-BGRA-extension-in-bindTexture.patch \
"

DEPENDS += "qtbase-native freetype jpeg libpng zlib openssl glib-2.0 ${ICU} udev ${XCB_DEPENDS} ${GL_DEPENDS} ${TSLIB_DEPENDS}"

# to provide xcb/xcb_icccm.h, xcb/xcb_image.h, xcb/xcb_keysyms.h and dbus/dbus.h
# XCB_DEPENDS += "xcb-util-wm xcb-util-image xcb-util-keysyms dbus"
# reenable when we switch to "-qt-xcb" and build of 
# QT_XCB = "-qt-xcb"
# src/platformsupport/glxconvenience/ is resolved to be before build of
# src/plugins/platforms/xcb/

XCB_DEPENDS ?= ""
QT_XCB ?= "-no-xcb"

QT_DISTRO_FLAGS ?= "-no-accessibility -no-sm"
QT_DISTRO_FLAGS_linuxstdbase = "-sm"

# Some can be used only for certain QT_MODULEs, so define them here, 
# but add them to QT_CONFIG_FLAGS e.g. in qtbase.inc
QT_SQL_DRIVER_FLAGS ?= "-no-sql-ibase -no-sql-mysql -no-sql-psql -no-sql-odbc -plugin-sql-sqlite"

# if you want to enable egl/gles2 in your .bbappend, don't forget to add right dependencies, e.g.
# GL_DEPENDS = "virtual/libgles2 virtual/libegl"
# QT_GLFLAGS = "-opengl es2 -eglfs"
GL_DEPENDS ?= "virtual/libgl"
QT_GLFLAGS ?= "-opengl"

# if you want to enable tslib in your .bbappend, don't forget to add tslib dependencies
# TSLIB_DEPENDS = "tslib"
# QT_TSLIB = "-tslib"
TSLIB_DEPENDS ?= ""
QT_TSLIB ?= "-no-tslib"

QT_WIDGETS ?= "-widgets"
QT_XML ?= "-xmlpatterns"
QT_WEBKIT ?= "-webkit"
QT_PHONON ?= "-phonon"
QT_DBUS ?= "-qdbus"
QT_MULTIMEDIA ?= "${@base_contains('DISTRO_FEATURES', 'pulseaudio', '-pulseaudio', '-no-pulseaudio', d)}"
QT_NAS ?= "-no-nas-sound"
QT_NIS ?= "-no-nis"
QT_CUPS ?= "-no-cups"
QT_STL ?= "-stl"
QT_SYSTEM_LIBS ?= "-system-libjpeg -system-libpng -system-zlib"
QT_TESTS ?= "-nomake tests"
QT_EXAMPLES ?= "-nomake examples"
QT_RELEASE ?= "-release"

QT_CONFIG_FLAGS += " \
    ${QT_RELEASE} \
    -reduce-relocations \
    -shared \
    -silent \
    -glib \
    -no-pch \
    -no-rpath \
    -pkg-config \
    ${QT_WIDGETS} \
    ${QT_SYSTEM_LIBS} \
    ${QT_NIS} \
    ${QT_CUPS} \
    ${QT_SQL_DRIVER_FLAGS} \
    ${QT_DISTRO_FLAGS} \
    ${QT_XCB} \
    ${QT_GLFLAGS} \
    ${QT_TESTS} \
    ${QT_EXAMPLES} \
    ${QT_TSLIB} \
"

INC_PR = "r2"

# Qt uses atomic instructions not supported in thumb mode
ARM_INSTRUCTION_SET = "arm"

do_generate_qt_config_file_append() {
    cat >> ${QT_CONF_PATH} <<EOF

[EffectivePaths]
Prefix=..
EOF
}

# qtbase is exception, we need to use mkspecs from ${S}
QMAKE_MKSPEC_PATH = "${B}"

# another exception is that we need to run bin/qmake, because EffectivePaths are relative to qmake location
OE_QMAKE_QMAKE_ORIG = "${STAGING_BINDIR_NATIVE}/${QT_DIR_NAME}/qmake"
OE_QMAKE_QMAKE = "bin/qmake"

# qtbase is exception, configure script is using our get(X)QEvalMakeConf and setBootstrapEvalVariable functions to read it from shell
export OE_QMAKE_COMPILER
export OE_QMAKE_CC
export OE_QMAKE_CFLAGS
export OE_QMAKE_CXX
export OE_QMAKE_CXXFLAGS
export OE_QMAKE_LINK
export OE_QMAKE_LDFLAGS
export OE_QMAKE_AR
export OE_QMAKE_STRIP

do_configure() {
    # we need symlink in path relative to source, because
    # EffectivePaths:Prefix is relative to qmake location
    if [ ! -e ${B}/bin/qmake ]; then
        mkdir ${B}/bin
        ln -sf ${OE_QMAKE_QMAKE_ORIG} ${B}/bin/qmake
    fi

    ${S}/configure -v \
        -dont-process \
        -opensource -confirm-license \
        -sysroot ${STAGING_DIR_TARGET} \
        -no-gcc-sysroot \
        -prefix ${OE_QMAKE_PATH_PREFIX} \
        -bindir ${OE_QMAKE_PATH_BINS} \
        -libdir ${OE_QMAKE_PATH_LIBS} \
        -datadir ${OE_QMAKE_PATH_DATA} \
        -sysconfdir ${OE_QMAKE_PATH_SETTINGS} \
        -docdir ${OE_QMAKE_PATH_DOCS} \
        -headerdir ${OE_QMAKE_PATH_HEADERS} \
        -archdatadir ${OE_QMAKE_PATH_ARCHDATA} \
        -libexecdir ${OE_QMAKE_PATH_LIBEXECS} \
        -plugindir ${OE_QMAKE_PATH_PLUGINS} \
        -importdir ${OE_QMAKE_PATH_IMPORTS} \
        -qmldir ${OE_QMAKE_PATH_QML} \
        -translationdir ${OE_QMAKE_PATH_TRANSLATIONS} \
        -testsdir ${OE_QMAKE_PATH_TESTS} \
        -examplesdir ${OE_QMAKE_PATH_EXAMPLES} \
        -hostbindir ${OE_QMAKE_PATH_HOST_BINS} \
        -hostdatadir ${OE_QMAKE_PATH_HOST_DATA} \
        -external-hostbindir ${OE_QMAKE_PATH_EXTERNAL_HOST_BINS} \
        -platform ${OE_QMAKESPEC} \
        -xplatform linux-oe-g++ \
        ${QT_CONFIG_FLAGS}

    qmake5_base_do_configure
}

do_compile_append() {
    # Build qmake for the target arch
    # Disable for now, because doesn't work well with separate ${B}
    # cp -ra ${S}/qmake ${B}
    # cd ${B}/qmake
    # ${OE_QMAKE_QMAKE}
    # Fix to use headers in ${B}
    # sed '/INCPATH/s#${S}#${B}#g' -i Makefile
    # oe_runmake CC="${CC}" CXX="${CXX}"
    # cd ${B}
}

do_install_append() {
    ### Fix up the binaries to the right location
    ### TODO: FIX
    # qemu built on host breaks do_package, remove it here (will be replaces with right qemu when do_compile is fixed
    # ERROR: objcopy failed with exit code 1 (cmd was 'arm-oe-linux-gnueabi-objcopy' --only-keep-debug '/OE/oe-core/tmp-eglibc/work/armv5te-oe-linux-gnueabi/qtbase/5.0.1-r0.0/package/usr/bin/qmake' '/OE/oe-core/tmp-eglibc/work/armv5te-oe-linux-gnueabi/qtbase/5.0.1-r0.0/package/usr/bin/.debug/qmake')
    rm -f ${D}/${bindir}/${QT_DIR_NAME}/qmake
    # install fonts manually if they are missing
    if [ ! -d ${D}/${OE_QMAKE_PATH_LIBS}/fonts ]; then
        cp -a ${S}/lib/fonts ${D}/${OE_QMAKE_PATH_LIBS}
    fi
}

PACKAGES =. " \
    ${PN}-fonts \
    ${PN}-fonts-ttf-vera \
    ${PN}-fonts-ttf-dejavu \
    ${PN}-fonts-pfa \
    ${PN}-fonts-pfb \
    ${PN}-fonts-qpf \
"

RRECOMMENDS_${PN}-fonts = " \
    ${PN}-fonts-ttf-vera \
    ${PN}-fonts-ttf-dejavu \
    ${PN}-fonts-pfa \
    ${PN}-fonts-pfb \
    ${PN}-fonts-qpf \
"

ALLOW_EMPTY_${PN}-fonts = "1"

FILES_${PN}-fonts-ttf-vera       = "${OE_QMAKE_PATH_LIBS}/fonts/Vera*.ttf"
FILES_${PN}-fonts-ttf-dejavu     = "${OE_QMAKE_PATH_LIBS}/fonts/DejaVu*.ttf"
FILES_${PN}-fonts-pfa            = "${OE_QMAKE_PATH_LIBS}/fonts/*.pfa"
FILES_${PN}-fonts-pfb            = "${OE_QMAKE_PATH_LIBS}/fonts/*.pfb"
FILES_${PN}-fonts-qpf            = "${OE_QMAKE_PATH_LIBS}/fonts/*.qpf*"
FILES_${PN}-fonts                = "${OE_QMAKE_PATH_LIBS}/fonts/README \
                                    ${OE_QMAKE_PATH_LIBS}/fonts/fontdir"
