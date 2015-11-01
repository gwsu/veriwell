
Veriwell 3
------------
Veriwell is a verilog simulator. It is compliant to verilog-95, but does not fully support it.

This package also integrates support for dumping lxt waveform files.
These files can be used with the gtkwaves waveform viewer.

Veriwell 3 is forked from the Veriwell.
Based on the origin project, Veriwell 3 adds some verilog-2001 features and fix some bugs.
See release log below.


Web
------------
Binary release (WINDOWS XP/7) :

http://pan.baidu.com/s/1gdJd4wN#path=%252Farts%252FVeriwell%25203

Open source Git :

https://github.com/balanx/veriwell

Origin project :

http://sourceforge.net/projects/veriwell


Installation
------------
WINDOWS XP/7

    This is binary release, so need NOT installation.
    Just add the location of directory 'veriwell3/' into 'PATH', the environment variable in the WINDOWS.


Linux/Cygwin

    tar -zxvf veriwell.tar.gz

    cd veriwell

    ./configure

    make

    make install


AMD64 Support

    For AMD64 targets the --enable-m32 switch may be used when running configure to force a 32 bit version.
    Note specifing this switch is equivilent to running ./configure CFLAGS=-m32 LDFLAGS=-m32.

    Lxt support for Gtkwaves (1.*) requires libz to be installed.
    Lxt support may be disabled by using the configuration switch --disable-lxt.


Usage
------------
It's very similar to Verilog-XL, so it will be very friendly if you are a long-term experienced IC engineer.
Try the following in the DOS command window.

    veriwell

    veriwell  a.v  b.v  c.v

    veriwell  tb.v  +libext+.v  -y dir1  -y dir2

    veriwell  -f  sim.f



Release Log
------------
2015-10-30 :

- add : Power (**) operator
- add : generate-if
- add : generate-case
- mod : disable specify-block
- add : bit-select for 2-dimensional array
- fix : task enable
- fix : searching top module
- add : non-declared wire
- fix : multi-modules in one file
- add : permit variable in 'delay control'
- add : permit function reference in parameter declaration
- add : $fscanf(), $sscanf()
- add : block-declaration in module


2014-10-9 :

- add : port definition in verilog-2001
- add : named parameter
- add : parameter declaration before port definition
- add : part-select operator '+:' and '-:'
- add : sensitive list wildcard
- add : `ifndef support
- add : reg initialization like 'reg a = 0, b = 0;'
- add : (* synthesis comments *) support
- fix : '-y' option


Enjoy !
