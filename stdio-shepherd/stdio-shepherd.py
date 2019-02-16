#!/usr/bin/python3

import csv
import fileinput
import glob
import io
import os
import re
import signal
import string
import sys
import time

from threading import Thread

sys.path.append( '/home/lumen/Volumetric/printrun' )
import printer

csv.register_dialect( 'StdioPrinterDriverCustom', delimiter = ' ', doublequote = False, escapechar = '\\', lineterminator = '\n', quotechar = '"', quoting = csv.QUOTE_MINIMAL, skipinitialspace = False )

class QuitNowException( Exception ):
    pass

class StdioPrinterDriver( ):

    def __init__( self ):
        self.printer      = printer.printer( self.printer_onlineCallback, self.printer_offlineCallback, self.printer_positionCallback, self.printer_temperatureCallback, self.printer_receiveCallback, self.printer_sendCallback )
        self.printProcess = printer.printprocess( self.printer, self.printProcess_showImageCallback, self.printProcess_hideImageCallback, self.printProcess_startedPrintingCallback, self.printProcess_finishedPrintingCallback )
        self.isOnline     = False
        self.isPrinting   = False
        self.reader       = csv.reader( sys.stdin,  csv.get_dialect( 'StdioPrinterDriverCustom' ) )
        self.writer       = csv.writer( sys.stdout, csv.get_dialect( 'StdioPrinterDriverCustom' ) )
        self.verbMap      = {
            'move':          self.move,
            'moveTo':        self.moveTo,
            'home':          self.home,
            'lift':          self.lift,
            'askTemp':       self.askTemp,
            'send':          self.send,
            'queryOnline':   self.queryOnline,
            'queryPrinting': self.queryPrinting,
            'stopPrinting':  self.stopPrinting,
            'terminate':     self.terminate
        }

    ##
    ## Callbacks for printer instance
    ##

    def printer_onlineCallback( self ):
        self.isOnline = True
        self.writer.writerow( [ 'printer_online' ] )
        sys.stdout.flush( )

    def printer_offlineCallback( self ):
        self.isOnline = False
        self.writer.writerow( [ 'printer_offline' ] )
        sys.stdout.flush( )

    def printer_positionCallback( self, position ):
        print( "+ printer_positionCallback, position: %.3f mm (raw: %s)" % ( float( position ), position ), file = sys.stderr )
        self.writer.writerow( [ 'printer_position', position ] )
        sys.stdout.flush( )

    def printer_temperatureCallback( self, temperatureInfo ):
        print( "+ printer_temperatureCallback, temperatureInfo: raw: %s" % temperatureInfo, file = sys.stderr )
        self.writer.writerow( [ 'printer_temperature', temperatureInfo ] )
        sys.stdout.flush( )

    def printer_receiveCallback( self, line ):
        self.writer.writerow( [ 'from_printer', line ] )
        sys.stdout.flush( )

    def printer_sendCallback( self, line ):
        self.writer.writerow( [ 'to_printer', line ] )
        sys.stdout.flush( )

    ##
    ## Callbacks for printprocess instance
    ##

    def printProcess_showImageCallback( self, fileName, brightness, index, total ):
        print( "+ printProcess_showImageCallback: file name %s, brightness %s, index %s, total %s" % ( fileName, brightness, index, total ), file = sys.stderr )
        self.writer.writerow( [ 'printProcess_showImage', fileName, brightness, index, total ] )
        sys.stdout.flush( )

    def printProcess_hideImageCallback( self ):
        self.writer.writerow( [ 'printProcess_hideImage' ] )
        sys.stdout.flush( )

    def printProcess_startedPrintingCallback( self ):
        self.isPrinting = True
        self.writer.writerow( [ 'printProcess_startedPrinting' ] )
        sys.stdout.flush( )

    def printProcess_finishedPrintingCallback( self ):
        self.isPrinting = False
        self.writer.writerow( [ 'printProcess_finishedPrinting' ] )
        sys.stdout.flush( )

    ##
    ## Methods
    ##

    def connect( self ):
        return self.printer.connect( )

    def disconnect( self ):
        if self.isPrinting:
            self.stopPrint( )
        self.printer.disconnect( )

    def stopPrint( self ):
        if not self.isPrinting:
            return
        self.printProcess.stop( )

    ##
    ## Verbs
    ##

    def move( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'move', 'not online', *args ]
        self.printer.move( *map( float, args[0:1] ) )
        return [ 'ok', 'move', *args ]

    def moveTo( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'moveTo', 'not online', *args ]
        self.printer.moveto( *map( float, args[0:1] ) )
        return [ 'ok', 'moveTo', *args ]

    def home( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'home', 'not online' ]
        self.printer.home( )
        return [ 'ok', 'home' ]

    def lift( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'lift', 'not online', *args ]
        self.printer.lift( *map( float, args[0:2] ) )
        return [ 'ok', 'lift', *args ]

    def askTemp( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'askTemp', 'not online' ]
        self.printer.asktemp( )
        return [ 'ok', 'askTemp' ]

    def send( self, *args ):
        if not self.isOnline or len( args ) == 0:
            return [ 'fail', 'send', 'not online', *args ]
        self.printer.send( args[0] )
        return [ 'ok', 'send', *args ]

    def queryOnline( self, *args ):
        return [ 'ok', 'queryOnline', 'true' if self.isOnline else 'false' ]

    def queryPrinting( self, *args ):
        return [ 'ok', 'queryPrinting', 'true' if self.isPrinting else 'false' ]

    def stopPrinting( self, *args ):
        self.stopPrint( )
        return [ 'ok', 'stopPrinting' ]

    def terminate( self, *args ):
        self.writer.writerow( [ 'ok', 'terminate' ] )
        sys.stdout.flush( )
        raise QuitNowException( )

    ##
    ## Main I/O loop
    ##

    def processInput( self ):
        self.writer.writerow( [ 'ok','started' ] )
        sys.stdout.flush( )

        for line in self.reader:
            if not ( line[0] in self.verbMap ):
                resultRow = [ 'unknown', line[0] ]
            else:
                try:
                    resultRow = self.verbMap[line[0]]( *line[1:] )
                    self.writer.writerow( resultRow )
                except QuitNowException:
                    return
                except:
                    print( "+ Caught an exception.", file = sys.stderr )

            sys.stdout.flush( )

##
## Main
##

signal.signal( signal.SIGHUP,  signal.SIG_IGN )
signal.signal( signal.SIGINT,  signal.SIG_IGN )
signal.signal( signal.SIGQUIT, signal.SIG_IGN )

driver = StdioPrinterDriver( )
if not driver.connect( ):
    driver.writer.writerow( [ 'warning', 'Couldn\'t open any serial device' ] )
else:
    driver.writer.writerow( [ 'info', 'port', driver.printer.port ] )
sys.stdout.flush( )

driver.processInput( )

driver.disconnect( )
