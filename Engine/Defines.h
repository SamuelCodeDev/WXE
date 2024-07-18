#ifndef DEFINES_H
#define DEFINES_H

#ifdef _WIN32
	#ifndef STRICT
		#define STRICT
	#endif

	#define WIN32_LEAN_AND_MEAN
	#define WIN32_EXTRA_LEAN
	#define VC_EXTRALEAN
	#define NOCRYPT
	#define NOIME
	#define NOWINRES
	#define NOKEYSTATES
	#define NOSYSCOMMANDS
	#define NORASTEROPS
	#define NOSHOWWINDOW
	#define OEMRESOURCE
	#define NOATOM
	#define NOCLIPBOARD
	#define NOCOLOR
	#define NODFERWINDOWPOS
	#define NOCTLMGR
	#define NODRAWTEXT
	#define NOKERNEL
	#define NONLS
	#define NOMEMMGR
	#define NOMETAFILE
	#define NOMINMAX
	#define NOOPENFILE
	#define NOSCROLL
	#define NOSERVICE
	#define NOSOUND
	#define NOTEXTMETRIC
	#define NOWH
	#define NOIMAGE
	#define NOCOMM
	#define NOKANJI
	#define NOHELP
	#define NOPROFILER
	#define NODEFERWINDOWPOS
	#define NOMCX
	#define NOTAPE
	#define NOSYSPARAMS
	#define NOWINDOWSTATION
	#define NOTRACKMOUSEEVENT
	#define NOVIRTUALKEYCODES
	#define NODESKTOP
	#define NOSECURITY
	#define NOMDI
	//#define NOSYSPARAMSINFO
	//#define NOWINABLE
	//#define NOMENUS
	//#define NOWINOFFSETS
	//#define NOWINSTYLES
	//#define NOWINMESSAGES
	//#define NOGDI
	//#define NOUSER
#endif

#if _MSC_VER
	#define _CRT_SECURE_NO_DEPRECATE
	#define _CRT_SECURE_NO_WARNINGS
#endif

#endif