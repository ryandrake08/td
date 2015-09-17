#pragma once

#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C"
{
#endif

// return directory suitable for our unix socket
NSString* TournamentSocketDirectory(void);

#ifdef __cplusplus
}
#endif
