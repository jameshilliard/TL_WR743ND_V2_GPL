################################################################################
#
# xdriver_xf86-video-xgixp -- XGIXP video driver
#
################################################################################

XDRIVER_XF86_VIDEO_XGIXP_VERSION = 1.7.99.3
XDRIVER_XF86_VIDEO_XGIXP_SOURCE = xf86-video-xgixp-$(XDRIVER_XF86_VIDEO_XGIXP_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_XGIXP_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_VIDEO_XGIXP_AUTORECONF = NO
XDRIVER_XF86_VIDEO_XGIXP_DEPENDENCIES = xserver_xorg-server xproto_fontsproto xproto_randrproto xproto_renderproto xproto_xextproto xproto_xproto
XDRIVER_XF86_VIDEO_XGIXP_INSTALL_TARGET_OPT = DESTDIR=$(TARGET_DIR) install

$(eval $(call AUTOTARGETS,package/x11r7,xdriver_xf86-video-xgixp))
