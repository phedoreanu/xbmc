/*
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://xbmc.org
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

#include "EGLNativeTypeAmlogic.h"
#include "guilib/gui3d.h"
#include "utils/AMLUtils.h"
#include "utils/StringUtils.h"
#include "utils/log.h"

#include <stdlib.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <EGL/egl.h>
#include <EGL/fbdev_window.h>

#ifdef CLASSNAME
#undef CLASSNAME
#endif
#define CLASSNAME "CEGLNativeTypeAmlogic"

CEGLNativeTypeAmlogic::CEGLNativeTypeAmlogic()
{
  const char *env_framebuffer = getenv("FRAMEBUFFER");

  // default to framebuffer 0
  m_framebuffer_name = "fb0";
  if (env_framebuffer)
  {
    std::string framebuffer(env_framebuffer);
    std::string::size_type start = framebuffer.find("fb");
    m_framebuffer_name = framebuffer.substr(start);
  }
  m_nativeWindow = NULL;
}

CEGLNativeTypeAmlogic::~CEGLNativeTypeAmlogic()
{
}

bool CEGLNativeTypeAmlogic::CheckCompatibility()
{
  char name[256] = {0};
  std::string modalias = "/sys/class/graphics/" + m_framebuffer_name + "/device/modalias";

  aml_get_sysfs_str(modalias.c_str(), name, 255);
  CStdString strName = name;
  StringUtils::Trim(strName);
  if (strName == "platform:mesonfb")
    return true;
  return false;
}

void CEGLNativeTypeAmlogic::Initialize()
{
  aml_permissions();
  aml_cpufreq_min(true);
  aml_cpufreq_max(true);
  DisableFreeScale();
  return;
}
void CEGLNativeTypeAmlogic::Destroy()
{
  aml_cpufreq_min(false);
  aml_cpufreq_max(false);
  return;
}

bool CEGLNativeTypeAmlogic::CreateNativeDisplay()
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  m_nativeDisplay = EGL_DEFAULT_DISPLAY;
  return true;
}

bool CEGLNativeTypeAmlogic::CreateNativeWindow()
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
#if defined(_FBDEV_WINDOW_H_)
  fbdev_window *nativeWindow = new fbdev_window;
  if (!nativeWindow)
    return false;

  RESOLUTION_INFO res;
  GetPreferredResolution(&res);

  nativeWindow->width = res.iWidth;
  nativeWindow->height = res.iHeight;
  m_nativeWindow = nativeWindow;

  return true;
#else
  return false;
#endif
}

bool CEGLNativeTypeAmlogic::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  if (!nativeDisplay)
    return false;
  *nativeDisplay = (XBNativeDisplayType*) &m_nativeDisplay;
  return true;
}

bool CEGLNativeTypeAmlogic::GetNativeWindow(XBNativeWindowType **nativeWindow) const
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  if (!nativeWindow)
    return false;
  *nativeWindow = (XBNativeWindowType*) &m_nativeWindow;
  return true;
}

bool CEGLNativeTypeAmlogic::DestroyNativeDisplay()
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  return true;
}

bool CEGLNativeTypeAmlogic::DestroyNativeWindow()
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
#if defined(_FBDEV_WINDOW_H_)
  delete (fbdev_window*)m_nativeWindow, m_nativeWindow = NULL;
#endif
  return true;
}

bool CEGLNativeTypeAmlogic::GetNativeResolution(RESOLUTION_INFO *res) const
{
  char mode[256] = {0};
  aml_get_sysfs_str("/sys/class/display/mode", mode, 255);
  CLog::Log(LOGDEBUG, "%s::%s %s", CLASSNAME, __func__, mode);
  return aml_mode_to_resolution(mode, res);
}

bool CEGLNativeTypeAmlogic::SetNativeResolution(const RESOLUTION_INFO &res)
{
  CLog::Log(LOGNOTICE, "%s::%s to %dx%d@%f", CLASSNAME, __func__, res.iScreenWidth, res.iScreenHeight, res.fRefreshRate);
  switch((int)res.fRefreshRate) // floor the resolution, so 23.98 will be brought down to 23
  {
    case 24:
      switch(res.iScreenWidth)
      {
        case 1920:
          SetDisplayResolution("1080p24hz");
          break;
      }
      break;
    case 25:
    case 50:
      switch(res.iScreenWidth)
      {
        case 1280:
          SetDisplayResolution("720p50hz");
          break;
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("1080i50hz");
          else
            SetDisplayResolution("1080p50hz");
          break;
      }
      break;
    case 30:
    case 60:
      switch(res.iScreenWidth)
      {
        case 1280:
          SetDisplayResolution("720p");
          break;
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("1080i");
          else
            SetDisplayResolution("1080p");
          break;
      }
      break;
  }

  return true;
}

bool CEGLNativeTypeAmlogic::SetDisplayResolution(const char *resolution)
{
  CLog::Log(LOGNOTICE, "%s::%s to %s", CLASSNAME, __func__, resolution);
  CStdString mode = resolution;
  // switch display resolution
  aml_set_sysfs_str("/sys/class/display/mode", mode.c_str());

  RESOLUTION_INFO res;
  if(aml_mode_to_resolution(mode.c_str(), &res)) {
    CLog::Log(LOGDEBUG, "%s::%s to %dx%d", CLASSNAME, __func__, res.iWidth, res.iHeight);
    int fd0;
    std::string framebuffer = "/dev/" + m_framebuffer_name;

    if ((fd0 = open(framebuffer.c_str(), O_RDWR)) >= 0)
    {
      struct fb_var_screeninfo vinfo;
      if (ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0)
      {
         vinfo.xres = res.iWidth;
         vinfo.yres = res.iHeight;
         vinfo.xres_virtual = vinfo.xres;
         vinfo.yres_virtual = vinfo.yres*2;
         vinfo.bits_per_pixel = 32;
         vinfo.activate = FB_ACTIVATE_ALL;
         ioctl(fd0, FBIOPUT_VSCREENINFO, &vinfo);
      }
      close(fd0);
    }
  }

  return true;
}

bool CEGLNativeTypeAmlogic::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  char valstr[256] = {0};
  aml_get_sysfs_str("/sys/class/amhdmitx/amhdmitx0/disp_cap", valstr, 255);
  std::vector<std::string> probe_str;
  probe_str.push_back("720p23.98hz");
  probe_str.push_back("720p25hz");
  probe_str.push_back("720p50hz");
  probe_str.push_back("720p");
  probe_str.push_back("1080p23.98hz");
  probe_str.push_back("1080p24hz");
  probe_str.push_back("1080p25hz");
  probe_str.push_back("1080p30hz");
  probe_str.push_back("1080p50hz");
  probe_str.push_back("1080p");
  probe_str.push_back("1080i50hz");
  probe_str.push_back("1080i");

  resolutions.clear();
  RESOLUTION_INFO res;
  for (std::vector<std::string>::const_iterator i = probe_str.begin(); i != probe_str.end(); ++i)
  {
    if(aml_mode_to_resolution(i->c_str(), &res))
      resolutions.push_back(res);
  }
  return resolutions.size() > 0;
}

bool CEGLNativeTypeAmlogic::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  // check display/mode, it gets defaulted at boot
  if (!GetNativeResolution(res))
  {
    // punt to 720p if we get nothing
    aml_mode_to_resolution("720p", res);
  }

  return true;
}

bool CEGLNativeTypeAmlogic::ShowWindow(bool show)
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  std::string blank_framebuffer = "/sys/class/graphics/" + m_framebuffer_name + "/blank";
  aml_set_sysfs_int(blank_framebuffer.c_str(), show ? 0 : 1);
  return true;
}

void CEGLNativeTypeAmlogic::DisableFreeScale()
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  // turn off frame buffer freescale
  aml_set_sysfs_int("/sys/class/graphics/fb0/free_scale", 0);
  aml_set_sysfs_int("/sys/class/graphics/fb1/free_scale", 0);
}

bool CEGLNativeTypeAmlogic::IsHdmiConnected() const
{
  CLog::Log(LOGDEBUG, "%s::%s", CLASSNAME, __func__);
  char hpd_state[2] = {0};
  aml_get_sysfs_str("/sys/class/amhdmitx/amhdmitx0/hpd_state", hpd_state, 2);
  return hpd_state[0] == '1';
}
