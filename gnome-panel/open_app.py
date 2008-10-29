from ooldtp import *
from ldtp import *
from ldtputils import *
from time import *


from ubuntutesting.ubuntu import *

try:

    dataXml = LdtpDataFileParser(datafilename)    
    mnuItem = dataXml.gettagvalue("menuitem")[0]
    wName = dataXml.gettagvalue("windowname")[0]

    start_time = time()
    
    open_and_check_menu_item(mnuItem, wName)

    stop_time = time()
    
    elapsed = stop_time - start_time
    
    log ('elapsed_time: ' + str(elapsed), 'comment')
    
    # Wait a couple of seconds and try to close the application
    try:
        wait(2)
        app = Application(wName)

        close_type = dataXml.gettagvalue("closetype")
        close_name = dataXml.gettagvalue("closename")
        if len(close_type) > 0 and len(close_name) > 0:
            app.exit(close_type[0], close_name[0])
        else:
            app.exit()
    except:
        pass
            
except LdtpExecutionError, msg:
    raise

