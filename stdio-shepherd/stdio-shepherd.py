#!/usr/bin/python3

import csv
import fileinput
import glob
import io
import os
import re
import string
import sys
import time

from threading    import Thread

#_baseDirectory = '/home/icekarma/devel/work/VolumetricLumen'
_baseDirectory = '/home/lumen/Volumetric'
sys.path.append( _baseDirectory + '/printrun' )

import printer

csv.register_dialect( 'StdioPrinterDriverCustom', delimiter = ' ', doublequote = True, escapechar = '\\', lineterminator = '\n', quotechar = '"', quoting = csv.QUOTE_MINIMAL, skipinitialspace = False )

class StdioPrinterDriver( ):

    def __init__( self ):
        self.printer      = printer.printer( self.printer_onlineCallback, self.printer_offlineCallback, self.printer_positionCallback, self.printer_temperatureCallback )
        self.printProcess = printer.printprocess( self.printer, self.printProcess_showImageCallback, self.printProcess_hideImageCallback, self.printProcess_startedPrintingCallback, self.printProcess_finishedPrintingCallback )
        self.isOnline     = False
        self.isPrinting   = False
        self.reader       = csv.reader( sys.stdin,  csv.get_dialect( 'StdioPrinterDriverCustom' ) )
        self.writer       = csv.writer( sys.stdout, csv.get_dialect( 'StdioPrinterDriverCustom' ) )
        self.verbMap      = {
            'move':    self.move,
            'moveTo':  self.moveTo,
            'home':    self.home,
            'lift':    self.lift,
            'askTemp': self.askTemp,
            'send':    self.send
        }

    ##
    ## Callbacks for printer instance
    ##

    def printer_onlineCallback( self ):
        self.isOnline = True
        print( "+ printer_online", file = sys.stderr )
        self.writer.writerow( [ 'printer_online' ] )

    def printer_offlineCallback( self ):
        self.isOnline = False
        print( "+ printer_offlineCallback", file = sys.stderr )
        self.writer.writerow( [ 'printer_offline' ] )

    def printer_positionCallback( self, position ):
        print( "+ printer_positionCallback, position: %.3f mm (raw: %s)" % ( float( position ), position ), file = sys.stderr )
        self.writer.writerow( [ 'printer_position', position ] )

    def printer_temperatureCallback( self, temperatureInfo ):
        print( "+ printer_temperatureCallback, temperatureInfo: raw: %s" % temperatureInfo, file = sys.stderr )
        self.writer.writerow( [ 'printer_temperature', temperatureInfo ] )

    ##
    ## Callbacks for printprocess instance
    ##

    def printProcess_showImageCallback( self, fileName, brightness, index, total ):
        print( "+ printProcess_showImageCallback: file name %s, brightness %s, index %s, total %s" % ( fileName, brightness, index, total ), file = sys.stderr )
        self.writer.writerow( [ 'printProcess_showImage', fileName, brightness, index, total ] )

    def printProcess_hideImageCallback( self ):
        print( "+ printProcess_hideImageCallback", file = sys.stderr )
        self.writer.writerow( [ 'printProcess_hideImage' ] )

    def printProcess_startedPrintingCallback( self ):
        self.isPrinting = True
        print( "+ printProcess_startedPrintingCallback", file = sys.stderr )
        self.writer.writerow( [ 'printProcess_startedPrinting' ] )

    def printProcess_finishedPrintingCallback( self ):
        self.isPrinting = False
        print( "+ printProcess_finishedPrintingCallback", file = sys.stderr )
        self.writer.writerow( [ 'printProcess_finishedPrinting' ] )

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

    def move( self, args = [ ] ):
        if not self.isOnline:
            return False

        if len( args ):
            self.printer.move( float( args[0] ) )
        else:
            self.printer.move( )

        return True

    def moveTo( self, args = [ ] ):
        if not self.isOnline:
            return False

        if len( args ):
            self.printer.moveto( float( args[0] ) )
        else:
            self.printer.moveto( )

        return True

    def home( self, args = [ ] ):
        if not self.isOnline:
            return False

        self.printer.home( )

        return True

    def lift( self, args = [ ] ):
        if not self.isOnline:
            return False

        if len( args ) > 1:
            self.printer.lift( float( args[0] ), float( args[1] ) )
        elif len( args ) > 0:
            self.printer.lift( float( args[0] ) )
        else:
            self.printer.lift( )

        return True

    def askTemp( self, args = [ ] ):
        if not self.isOnline:
            return False

        self.printer.asktemp( )

        return True

    def send( self, args = [ ] ):
        if not self.isOnline or len( args ) == 0:
            return False

        self.printer.send( args[0] )

        return True

    def processInput( self ):
        for line in self.reader:
            print( "verb: |%s| args: |%s|" % ( line[0], '|'.join( line[1:] ) ), file = sys.stderr )

            if not ( line[0] in self.verbMap ):
                self.writer.writerow( [ 'unknown', line[0] ] )
                continue

            if self.verbMap[line[0]]( line[1:] if len( line ) > 0 else [ ] ):
                self.writer.writerow( [ 'ok', line[0] ] )
            else:
                self.writer.writerow( [ 'fail', line[0] ] )

##
## Main
##

driver = StdioPrinterDriver( )
if not driver.connect( ):
    print( "Warning: Couldn't open any serial device", file = sys.stderr )

driver.processInput( )
driver.disconnect( )
