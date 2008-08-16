require 'mkmf'
extension_name = 'qtvr'

dir_config("qtvr")

$INCFLAGS += " -I/System/Library/Frameworks/QuickTime.framework/Headers/ -I/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Headers/"
$LIBS += "-framework QuickTime"

create_makefile("qtvr")