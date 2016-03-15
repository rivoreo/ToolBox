#ifndef _WINDOWSNT_NATIVE
TOOL(cat)
#ifndef _NO_SELINUX
TOOL(chcon)
#endif
#endif
TOOL(chmod)
#ifndef _WINDOWSNT_NATIVE
TOOL(chown)
TOOL(chroot)
TOOL(clear)
#endif
TOOL(cmp)
TOOL(date)
#ifndef _WINDOWSNT_NATIVE
TOOL(dd)
#endif
TOOL(df)
#if !defined _WINDOWSNT_NATIVE && !defined __INTERIX
TOOL(dmesg)
#endif
#ifndef _WINDOWSNT_NATIVE
TOOL(du)
#endif
TOOL(exists)
#ifdef __linux__
#ifndef _NO_SELINUX
TOOL(getenforce)
#endif
TOOL(getevent)
#endif
TOOL(getopt)
TOOL(hd)
#ifndef _WINDOWSNT_NATIVE
TOOL(hostname)
TOOL(id)
#ifndef __INTERIX
TOOL(ifconfig)
TOOL(iftop)
#endif
#ifdef __linux__
TOOL(insmod)
#endif
#endif
TOOL(ioctl)
#ifndef _WINDOWSNT_NATIVE
#ifndef __INTERIX
TOOL(isptrace1allowed)
#endif
TOOL(kill)
#ifndef __INTERIX
TOOL(kill1)
#endif
#endif
TOOL(ln)
TOOL(ls)
#ifndef _WINDOWSNT_NATIVE
#ifdef __linux__
TOOL(lsmod)
#endif
TOOL(lsof)
#endif
TOOL(md5)
#ifndef _WINDOWSNT_NATIVE
TOOL(more)
#endif
TOOL(mkdir)
#ifndef _WINDOWSNT_NATIVE
TOOL(mknod)
#endif
TOOL(mkswap)
TOOL(modexe)
TOOL(modexeb)
#ifndef _WINDOWSNT_NATIVE
TOOL(mtdread)
#endif
TOOL(mv)
#ifndef _WINDOWSNT_NATIVE
#ifdef __linux__
TOOL(netstat)
#endif
TOOL(nohup)
#ifdef __linux__
TOOL(notify)
#endif
#endif
#if !defined __APPLE__ || !defined _SHARED 
TOOL(printenv)
#endif
#ifndef _WINDOWSNT_NATIVE
TOOL(ps)
#ifndef __INTERIX
TOOL(r)
#endif
#endif
TOOL(readlink)
#ifndef _WINDOWSNT_NATIVE
TOOL(readtty)
#endif
#ifndef __INTERIX
TOOL(reboot)
#endif
#ifndef _WINDOWSNT_NATIVE
TOOL(renice)
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
#ifndef _WINDOWSNT_NATIVE
TOOL(schedtop)
#endif
#ifdef __linux__
TOOL(sendevent)
#endif
#ifndef _WINDOWSNT_NATIVE
TOOL(service)
#endif
#ifdef __linux__
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
#if defined __linux__ || defined __FreeBSD__
TOOL(swapoff)
TOOL(swapon)
#endif
#ifndef _WINDOWSNT_NATIVE
#ifndef __INTERIX
TOOL(sync)
#endif
TOOL(tee)
#endif
#if !defined _WINDOWSNT_NATIVE && !defined __APPLE__
TOOL(top)
#endif
TOOL(touch)
TOOL(uptime)
#if !defined __APPLE__ && !defined _WINDOWSNT_NATIVE && !defined __FreeBSD__ && !defined __INTERIX
TOOL(vmstat)
#endif
TOOL(which)
