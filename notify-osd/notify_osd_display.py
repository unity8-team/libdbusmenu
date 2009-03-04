# -*- coding: utf-8 -*-

import ldtp
import ldtputils

from time import time, gmtime, strftime

from desktoptesting.deskex import NotifyOSD
from desktoptesting.check import ScreenshotCompare, PASS, FAIL

test = NotifyOSD()

dataXml  = ldtputils.LdtpDataFileParser(datafilename)

oracle = dataXml.gettagvalue("oracle")[0]
summary = dataXml.gettagvalue("summary")[0]
body = dataXml.gettagvalue("body")[0]
icon = dataXml.gettagvalue("icon")[0]

test.open(False)
test.notify(summary, body, icon)
elapsed, screeny = test.grab_image_and_wait(summary)
test.exit()
    
testcheck = ScreenshotCompare(oracle, screeny)
check = testcheck.perform_test()

if check == FAIL:
    ldtp.log ('Screenshots differ.', 'cause')
    ldtp.log ('Screenshots differ.', 'error')

ldtp.log (str(elapsed), 'time')