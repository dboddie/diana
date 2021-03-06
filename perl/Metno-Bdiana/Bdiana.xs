#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "bdiana_capi.h"


MODULE = Metno::Bdiana		PACKAGE = Metno::Bdiana		

void
init_(OUTLIST int ret)
  PREINIT:
    char* argv[] =  {"-v", "-L", "/etc/diana/setup/log4cpp.properties"};
  CODE:
    ret = diana_init(3 , argv);

void
free()
  CODE:
    diana_dealloc();

void
readSetupFile(const char* setupFilename, OUTLIST int ret)
  CODE:
    ret = diana_readSetupFile(setupFilename);

void
parseAndProcessString(const char* str, OUTLIST int ret)
  CODE:
    ret = diana_parseAndProcessString(str);


