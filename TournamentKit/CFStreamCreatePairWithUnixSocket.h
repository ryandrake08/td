#pragma once

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFStream.h>

// Use CFStreamCreatePairWithSocket to create a UNIX domain socket and a CFReadStream and CFWriteStream to talk to them
void CFStreamCreatePairWithUnixSocket(CFAllocatorRef alloc, CFStringRef name, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream);
