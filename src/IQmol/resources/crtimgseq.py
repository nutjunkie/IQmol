#!/usr/bin/python

# author: Martin Michel
# eMail: martin.michel@macscripter.net
# created: 16.05.2008

# This script is called from an AppleScript and converts a list of
# image files into an image sequence (QuickTime movie) using the QTKit.

import sys
from AppKit import NSImage
from QTKit import *

def crtimagesequence(movpath, imgpaths, timeval, timescale):
    """I am creating a new image sequence (QuickTime movie)."""
    mov, err = QTMovie.alloc().initToWritableFile_error_(movpath, None)
    
    if mov == None:
        errmsg = "Could not create movie file: %s" % (movpath)
        raise IOError, errmsg
        
    duration = QTMakeTime(timeval, timescale)
    # you can also use "tiff"
    attrs = {QTAddImageCodecType: "jpeg"}
    
    for imgpath in imgpaths:
        img = NSImage.alloc().initWithContentsOfFile_(imgpath)
        mov.addImage_forDuration_withAttributes_(img, duration, attrs)
        mov.updateMovieFile()

if __name__ == '__main__':
    # file path to the new QuickTime movie
    movpath = sys.argv[1].decode('utf-8')


    # time value & scale to be used for creating a QTTime duration
    # timeval = float(sys.argv[2])
    #timescale = float(sys.argv[3])
    timeval   = 1.0
    # this is the number of frames per second:
    timescale = 15.0

    # creating a list of decoded image file paths
    #tmppaths = sys.argv[5:]
    tmppaths = sys.argv[2:]
    imgpaths = []
    for tmppath in tmppaths:
        imgpaths.append(tmppath.decode('utf-8'))
    
    imgpaths.sort()
    crtimagesequence(movpath, imgpaths, timeval, timescale)
