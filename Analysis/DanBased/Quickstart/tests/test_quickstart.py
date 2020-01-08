from dybtest import Run, Matcher
checks = {
    '.*FATAL':2,
    '.*\*\*\* Break \*\*\* segmentation violation':3,
    '.*IOError':4,
    '.*ValueError\:':5,
    '^\#\d':None
    }
Run.parser = Matcher( checks, verbose=False )
Run.opts = { 'maxtime':300 }
                      
def test_printrawdata_histref():
    cmd = "nuwa.py --history=off -A none -l 5 -n -1 -m Quickstart.PrintRawData share/sample.root "
    Run( cmd , histref="rawDataResult.root")()

def test_printrawdata_output():
    cmd = "nuwa.py --history=off -A none -l 3 -n 1 -m Quickstart.PrintRawData share/sample.root "
    Run( cmd , reference=True )()


