/*
 *      Copyright (C) 2011-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include <EGL/egl.h>
#include "EGLNativeTypeFbdev.h"
#include "utils/log.h"
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include "utils/StringUtils.h"
#include "guilib/gui3d.h"
#include <linux/media.h>

#include <unistd.h>

#include "utils/StringUtils.h"

#ifdef CLASSNAME
#undef CLASSNAME
#endif
#define CLASSNAME "CEGLNativeTypeFbdev"

CEGLNativeTypeFbdev::CEGLNativeTypeFbdev()
{
  m_iFBHandle = -1;
  m_nativeWindow  = NULL;
  m_nativeDisplay = NULL;
}

CEGLNativeTypeFbdev::~CEGLNativeTypeFbdev()
{
  if (m_nativeWindow)
    free(m_nativeWindow);

  if(m_iFBHandle >= 0)
  {
    close(m_iFBHandle);
    m_iFBHandle = -1;
  }
}

bool CEGLNativeTypeFbdev::CheckCompatibility()
{
  m_iFBHandle = open("/dev/fb0", O_RDWR, 0);
  if(m_iFBHandle < 0)
    return false;

  struct fb_var_screeninfo info;
  if(ioctl(m_iFBHandle, FBIOGET_VSCREENINFO, &info) == -1)
    return false;

  CLog::Log(LOGNOTICE, "%s::%s FBDev device: %d, info.xres %d info.yres %d info.upper_margin %d info.lower_margin %d info.pixclock %d",
    CLASSNAME, __func__, m_iFBHandle, info.xres, info.yres, info.upper_margin, info.lower_margin, info.pixclock);

  width = info.xres;
  height = info.yres;

  return true;
}

void CEGLNativeTypeFbdev::Initialize()
{
  return;
}
void CEGLNativeTypeFbdev::Destroy()
{
  return;
}

bool CEGLNativeTypeFbdev::CreateNativeDisplay()
{
  m_nativeDisplay = EGL_DEFAULT_DISPLAY;
  return true;
}

bool CEGLNativeTypeFbdev::CreateNativeWindow()
{
  fbdev_window *nativeWindow = new fbdev_window;
  if (!nativeWindow)
    return false;

  nativeWindow->width = width;
  nativeWindow->height = height;
  m_nativeWindow = nativeWindow;
  return true;
}

bool CEGLNativeTypeFbdev::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const
{
  if (!nativeDisplay)
    return false;
  *nativeDisplay = (XBNativeDisplayType*) &m_nativeDisplay;
  return true;
}

bool CEGLNativeTypeFbdev::GetNativeWindow(XBNativeWindowType **nativeWindow) const
{
  if (!nativeWindow)
    return false;
  *nativeWindow = (XBNativeWindowType*) &m_nativeWindow;
  return true;
}

bool CEGLNativeTypeFbdev::DestroyNativeDisplay()
{
  return true;
}

bool CEGLNativeTypeFbdev::DestroyNativeWindow()
{
  free(m_nativeWindow);
  return true;
}

bool CEGLNativeTypeFbdev::GetNativeResolution(RESOLUTION_INFO *res) const
{
  res->iWidth = width;
  res->iHeight = height;
  res->fRefreshRate = 60;
  res->dwFlags = D3DPRESENTFLAG_PROGRESSIVE;
  res->iScreen = 0;
  res->bFullScreen = true;
  res->iSubtitles = (int)(0.965 * res->iHeight);
  res->fPixelRatio = 1.0f;
  res->iScreenWidth = res->iWidth;
  res->iScreenHeight = res->iHeight;
  res->strMode = StringUtils::Format("%dx%d @ %.2f%s - Full Screen", res->iScreenWidth, res->iScreenHeight, res->fRefreshRate, res->dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");

  CLog::Log(LOGNOTICE, "Current resolution: %s", res->strMode.c_str());
  return true;
}

bool CEGLNativeTypeFbdev::SetNativeResolution(const RESOLUTION_INFO &res)
{
  struct fb_var_screeninfo info;
  if(ioctl(m_iFBHandle, FBIOGET_VSCREENINFO, &info) == -1)
    CLog::Log(LOGERROR, "%s::%s FBIOGET_VSCREENINFO unexpected error", CLASSNAME, __func__);

  info.reserved[0] = 0;
  info.reserved[1] = 0;
  info.reserved[2] = 0;
  info.xoffset = 0;
  info.yoffset = 0;
  info.activate = FB_ACTIVATE_NOW;

  info.bits_per_pixel = 32;
  info.red.offset     = 16;
  info.red.length     = 8;
  info.green.offset   = 8;
  info.green.length   = 8;
  info.blue.offset    = 0;
  info.blue.length    = 8;
  info.transp.offset  = 0;
  info.transp.length  = 0;

  info.xres = res.iScreenWidth;
  info.yres = res.iScreenHeight;

  info.yres_virtual = info.yres * 2;

  if (ioctl(m_iFBHandle, FBIOPUT_VSCREENINFO, &info) == -1)
  {
//    info.yres_virtual = info.yres;
    CLog::Log(LOGERROR, "%s::%s - FBIOPUT_VSCREENINFO error", CLASSNAME, __func__);
    return false;
  }

  if (ioctl(m_iFBHandle, FBIOPAN_DISPLAY, &info) == -1)
  {
    CLog::Log(LOGERROR, "%s::%s - FBIOPAN_DISPLAY error", CLASSNAME, __func__);
    return false;
  }

  CLog::Log(LOGNOTICE, "%s::%s width %d height %d refresh %f", CLASSNAME, __func__, res.iScreenWidth, res.iScreenHeight, res.fRefreshRate);

  return true;
}

bool CEGLNativeTypeFbdev::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  RESOLUTION_INFO res;
  if (GetNativeResolution(&res) && res.iWidth > 1 && res.iHeight > 1)
  {
    resolutions.push_back(res);
    return true;
  }
  return false;
}

bool CEGLNativeTypeFbdev::GetPreferredResolution(RESOLUTION_INFO *res) const
{
    GetNativeResolution(res);
    return true;
}

bool CEGLNativeTypeFbdev::ShowWindow(bool show)
{
  return false;
}
