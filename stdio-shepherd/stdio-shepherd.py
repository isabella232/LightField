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

sys.path.append( '/usr/share/lightfield/libexec/printrun' )

import printer

csv.register_dialect( 'StdioPrinterDriverCustom', delimiter = ' ', doublequote = False, escapechar = '\\', lineterminator = '\n', quotechar = '"', quoting = csv.QUOTE_MINIMAL, skipinitialspace = False )

class QuitNowException( Exception ):
    pass

class StdioPrinterDriver( ):

    def __init__( self ):
        self.printer      = printer.printer( self.printer_onlineCallback, self.printer_offlineCallback, self.printer_positionCallback, self.printer_temperatureCallback, self.printer_receiveCallback, self.printer_sendCallback )
        self.isOnline     = False
        self.isPrinting   = False
        self.reader       = csv.reader( sys.stdin,  csv.get_dialect( 'StdioPrinterDriverCustom' ) )
        self.writer       = csv.writer( sys.stdout, csv.get_dialect( 'StdioPrinterDriverCustom' ) )
        self.verbMap      = {
            'moveRel':   self.moveRel,
            'moveAbs':   self.moveAbs,
            'home':      self.home,
            'send':      self.send,
            'terminate': self.terminate
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
    ## Methods
    ##

    def connect( self ):
        return self.printer.connect( )

    def disconnect( self ):
        if self.isPrinting:
            self.stopPrint( )
        self.printer.disconnect( )

    ##
    ## Verbs
    ##

    def moveRel( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'moveRel', 'not online', *args ]
        self.printer.move( *map( float, args[0:2] ) )
        return [ 'ok', 'moveRel', *args ]

    def moveAbs( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'moveAbs', 'not online', *args ]
        self.printer.moveto( *map( float, args[0:2] ) )
        return [ 'ok', 'moveAbs', *args ]

    def home( self, *args ):
        if not self.isOnline:
            return [ 'fail', 'home', 'not online' ]
        self.printer.home( )
        return [ 'ok', 'home' ]

    def send( self, *args ):
        if not self.isOnline or len( args ) == 0:
            return [ 'fail', 'send', 'not online', *args ]
        self.printer.send( args[0] )
        return [ 'ok', 'send', *args ]

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
