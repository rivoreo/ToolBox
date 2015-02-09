#ifndef _TOOLBOX_H
#define _TOOLBOX_H

#ifdef __cplusplus
extern "C" {
#endif

#define TOOL(name) int name##_main(int, char **);
#include <tools.h>
#undef TOOL

#ifdef __cplusplus
}
#endif

#endif
