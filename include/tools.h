#ifndef _WIN32_WNT_NATIVE
TOOL(cat)
#ifndef _NO_SELINUX
TOOL(chcon)
#endif
#endif
TOOL(chmod)
#ifndef _WIN32_WNT_NATIVE
TOOL(chown)
#endif
TOOL(cmp)
TOOL(date)
#ifndef _WIN32_WNT_NATIVE
TOOL(dd)
#endif
TOOL(df)
//#ifdef __linux__
#if !defined __APPLE__ && !defined _WIN32_WNT_NATIVE
TOOL(dmesg)
#endif
#ifndef _WIN32_WNT_NATIVE
TOOL(du)
#endif
TOOL(exists)
#ifndef _WIN32_WNT_NATIVE
#ifndef _NO_SELINUX
TOOL(getenforce)
#endif
#endif
#ifdef __linux__
TOOL(getevent)
#endif
TOOL(hd)
#ifndef _WIN32_WNT_NATIVE
TOOL(id)
TOOL(ifconfig)
TOOL(iftop)
#ifdef __linux__
TOOL(insmod)
#endif
TOOL(ioctl)
TOOL(isptrace1allowed)
TOOL(kill)
TOOL(kill1)
#endif
TOOL(ln)
TOOL(ls)
#ifndef _WIN32_WNT_NATIVE
#ifdef __linux__
TOOL(lsmod)
#endif
TOOL(lsof)
#endif
TOOL(md5)
TOOL(mkdir)
TOOL(mkswap)
TOOL(modexe)
TOOL(modexeb)
#ifndef _WIN32_WNT_NATIVE
TOOL(mtdread)
#endif
TOOL(mv)
#ifndef _WIN32_WNT_NATIVE
TOOL(netstat)
TOOL(nohup)
#ifdef __linux__
TOOL(notify)
#endif
#endif
#if !defined __APPLE__ || !defined _SHARED 
TOOL(printenv)
#endif
#ifndef _WIN32_WNT_NATIVE
TOOL(ps)
TOOL(r)
TOOL(readtty)
#ifndef __APPLE__
TOOL(renice)
#endif
#ifndef _NO_SELINUX
TOOL(restorecon)
#endif
TOOL(rm)
#endif
TOOL(rmdir)
#ifdef __linux__
TOOL(rmmod)
TOOL(rotatefb)
TOOL(route)
#ifndef _NO_SELINUX
TOOL(runcon)
#endif
#endif
#ifndef _WIN32_WNT_NATIVE
TOOL(schedtop)
#endif
#ifdef __linux__
TOOL(sendevent)
TOOL(setconsole)
#ifndef _NO_SELINUX
TOOL(setenforce)
#endif
TOOL(setkey)
#ifndef _NO_SELINUX
TOOL(setsebool)
#endif
#endif
TOOL(sleep)
#ifdef __linux__
TOOL(swapoff)
TOOL(swapon)
#endif
#ifndef _WIN32_WNT_NATIVE
TOOL(sync)
TOOL(top)
#endif
TOOL(touch)
TOOL(uptime)
#if !defined __APPLE__ && !defined _WIN32_WNT_NATIVE
TOOL(vmstat)
#endif
