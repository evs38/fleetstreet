.* ****************************************************************************
.* * FleetStreet Development Kit                                              *
.* ****************************************************************************

:userdoc.
:docprof toc=123.
:title.FleetStreet Development Kit Rev. 2

:h1.Introduction
:p.
:lines align=center.
FleetStreet
Development Kit
Rev. 2 (25 June 1996)
:elines.
:p.
This file describes interfaces to extend the functionality of FleetStreet.
:p.
Currently the following interfaces are available&colon.
:sl compact.
:li.:link reftype=hd refid=echoman.Echo manager DLLs:elink.
:esl.
:p.
If you have technical questions you can reach the author at the following
addresses&colon.
:parml break=none compact tsize=20.
:pt.:hp2.Fidonet:ehp2.
:pd.Michael Hohner 2&colon.2490/2520.17
:pt.:hp2.Internet:ehp2.
:pd.miho@n-online.de
:eparml.

:h1 id=echoman.Echo manager DLLs
:p.Echo manager DLLs are loaded and called by FleetStreet when echo areas
are linked or unlinked.
:p.It's the purpose of the DLL to add the echo area definition to the tosser
configuration file or to remove it respectively.
:p.The DLL must have the following named entry points&colon.
:sl compact.
:li.:link reftype=hd refid=dll01.QueryVersion:elink.
:li.:link reftype=hd refid=dll02.QueryParamBlockSize:elink.
:li.:link reftype=hd refid=dll03.SetupParams:elink.
:li.:link reftype=hd refid=dll04.AddEcho:elink.
:li.:link reftype=hd refid=dll05.RemoveEcho:elink.
:esl.
:p.
All the above entry points must be present in the DLL, otherwise FleetStreet
will reject the DLL. FleetStreet uses the :link reftype=fn refid=syscall.:hp2._System:ehp2.:elink.
calling convention for calling the functions.

:h2.Additional parameters
:p.The DLL may require additional parameters for adding or deleting echo area entries.
These can be configured when the user presses the :hp2.Configure:ehp2. button in the
echo manager settings notebook.
:p.The memory for storing the parameters is allocated (and freed) by FleetStreet. The
parameter block is also stored in FleetStreet's INI files. The DLL need not care about
storing and loading the parameter block.
:p.
The parameter block is a structure defined by the DLL. The first 16 bytes of the
parameter block forms a signature. Before the DLL uses the parameter block, it
must first check that the signature in the parameter block matches its own.
When it doesn't, the DLL must reject the parameter block.
:p.
When FleetStreet allocates a new parameter block, it sets all bytes in the block
to Zero. When the DLL recognizes that the first byte in the parameter
block is 0, it must not reject the parameter block, but initialize it. The DLL
must insert the signature and set all other parameters to default values.

:h2 id=dll01.QueryVersion
:p.
:hp2.Syntax:ehp2.
:xmp.
ULONG APIENTRY QueryVersion(VOID);
:exmp.
:p.
:hp2.Description:ehp2.
:p.
FleetStreet calls this function after loading the DLL. The DLL must return its compile time
version number, which is defined in ECHOMAN.H (DLL_VERSION).
:p.
When the returned value differs from what FleetStreet expects, the DLL is unloaded
again. No other DLL functions are called.
:p.
:hp2.Parameters:ehp2.
:p.
none.
:p.
:hp2.Return values:ehp2.
:p.
Compile time version number. Use DLL_VERSION from ECHOMAN.H.

:h2 id=dll02.QueryParamBlockSize
:p.
:hp2.Syntax:ehp2.
:xmp.
ULONG APIENTRY QueryParamBlockSize(VOID);
:exmp.
:p.
:hp2.Description:ehp2.
:p.
FleetStreet calls this function to query the size of the parameter block
that is used by the DLL to store additional parameters. FleetStreet uses
the returned value to allocate memory for the parameter block and to
compare the size with that of a already allocated parameter block.
:p.
The returned value must be MIN_PARAM_SIZE or bigger.
:p.
:hp2.Parameters:ehp2.
:p.
none.
:p.
:hp2.Return values:ehp2.
:p.
Size of the DLL's parameter block.

:h2 id=dll03.SetupParams
:p.
:hp2.Syntax:ehp2.
:xmp.
ULONG APIENTRY SetupParams(PVOID   pParamBlock,
                           ULONG   ulParamBlockSize,
                           HWND    hwndOwner,
                           HAB     hab,
                           HMODULE hModule);
:exmp.
:p.
:hp2.Description:ehp2.
:p.
FleetStreet calls this function when the user presses the :hp2.Configure:ehp2.
button in the echo manager settings notebook. The DLL should then
display a dialog window to let the user set additional parameters, e.g.
default path for new echo areas, default parameters etc.
:p.
The DLL must check the signature in the parameter block. When the parameter
block isn't already initialized, the DLL must at least set the signature in
the parameter block. It should also set the parameters to default values.
:p.
:hp2.Parameters:ehp2.
:parml break=none tsize=25.
:pt.:hp2.pParamBlock:ehp2.
:pd.Pointer to the parameter block. The memory is allocated by FleetStreet.
:pt.:hp2.ulParamBlockSize:ehp2.
:pd.Size of the memory block pointed to by pParamBlock. The DLL must return
ECHOMAN_PARAMSIZE if the size doesn't match the size of its own parameter
block.
:pt.:hp2.hwndOwner:ehp2.
:pd.Window handle that can be used as owner window handle when displaying
a dialog window.
:pt.:hp2.hab:ehp2.
:pd.Anchor block; may be used for loading resources etc.
:pt.:hp2.hModule:ehp2.
:pd.Module handle of the DLL; may be used for loading resources.
:eparml.
:p.
:hp2.Return values:ehp2.
:p.
:sl compact.
:li.ECHOMAN_OK
:li.ECHOMAN_PARAMSIZE
:li.ECHOMAN_FORMAT
:li.ECHOMAN_CANCEL
:li.ECHOMAN_ERROR
:esl.

:h2 id=dll04.AddEcho
:p.
:hp2.Syntax:ehp2.
:xmp.
ULONG APIENTRY AddEcho(PVOID pParamBlock,
                       ULONG ulParamBlockSize,
                       PCHAR pchCfgFile,
                       PCHAR pchCurrentAddress,
                       PCHAR pchUplinkAddress,
                       PCHAR pchAreaTag,
                       ULONG ulFlags);
:exmp.
:p.
:hp2.Description:ehp2.
:p.
FleetStreet calls this function when the user links a new echo area. The DLL
can then add the echo area definition to the Tosser configuration file.
:p.
FleetStreet calls the function from a secondary thread; the DLL need not start
a separate thread to do disk-IO.
:p.
:hp2.Parameters:ehp2.
:parml break=none tsize=25.
:pt.:hp2.pParamBlock:ehp2.
:pd.Pointer to the parameter block. The memory is allocated by FleetStreet.
:pt.:hp2.ulParamBlockSize:ehp2.
:pd.Size of the memory block pointed to by pParamBlock. The DLL must return
ECHOMAN_PARAMSIZE if the size doesn't match the size of its own parameter
block.
:pt.:hp2.pchCfgFile:ehp2.
:pd.Pointer to a 0-terminated string containing the name of the
tosser configuration file. It's the same file name as the one in
the FleetStreet setup notebook. If the Tosser uses more than one
configuration file, the names of the other files must be deduced
from :hp2.pchCfgFile:ehp2..
:pt.:hp2.pchCurrentAddress:ehp2.
:pd.Address of the user that is to be used for the new area. It's a
0-terminated string containing the address in the form
:xmp.
Zone&colon.Net/Node[.Point]
:exmp.
:pt.:hp2.pchUplinkAddress:ehp2.
:pd.Address of the uplink for the new area. Same format as above.
:pt.:hp2.pchAreaTag:ehp2.
:pd.Area tag of the new area.
:pt.:hp2.ulFlags:ehp2.
:pd.Additional flags. Currently, this parameter is unused and always 0.
:eparml.
:p.
:hp2.Return values:ehp2.
:p.
:sl compact.
:li.ECHOMAN_OK
:li.ECHOMAN_PARAMSIZE
:li.ECHOMAN_FORMAT
:li.ECHOMAN_ERROR
:li.ECHOMAN_CFGNOTFOUND
:li.ECHOMAN_CFGREAD
:li.ECHOMAN_CFGWRITE
:li.ECHOMAN_CFGFORMAT
:li.ECHOMAN_ALREADYLINKED
:esl.

:h2 id=dll05.RemoveEcho
:p.
:hp2.Syntax:ehp2.
:xmp.
ULONG APIENTRY RemoveEcho(PVOID pParamBlock,
                          ULONG ulParamBlockSize,
                          PCHAR pchCfgFile,
                          PCHAR pchCurrentAddress,
                          PCHAR pchUplinkAddress,
                          PCHAR pchAreaTag,
                          ULONG ulFlags);
:exmp.
:p.
:hp2.Description:ehp2.
:p.
FleetStreet calls this function when the user unlinks from an echo area. The DLL
can then remove the echo area definition from the Tosser configuration file.
:p.
FleetStreet calls the function from a secondary thread; the DLL need not start
a separate thread to do disk-IO.
:p.
:hp2.Parameters:ehp2.
:parml break=none tsize=25.
:pt.:hp2.pParamBlock:ehp2.
:pd.Pointer to the parameter block. The memory is allocated by FleetStreet.
:pt.:hp2.ulParamBlockSize:ehp2.
:pd.Size of the memory block pointed to by pParamBlock. The DLL must return
ECHOMAN_PARAMSIZE if the size doesn't match the size of its own parameter
block.
:pt.:hp2.pchCfgFile:ehp2.
:pd.Pointer to a 0-terminated string containing the name of the
tosser configuration file. It's the same file name as the one in
the FleetStreet setup notebook. If the Tosser uses more than one
configuration file, the names of the other files must be deduced
from :hp2.pchCfgFile:ehp2..
:pt.:hp2.pchCurrentAddress:ehp2.
:pd.Address of the user that is to be used for the new area. It's a
0-terminated string containing the address in the form
:xmp.
Zone&colon.Net/Node[.Point]
:exmp.
:pt.:hp2.pchUplinkAddress:ehp2.
:pd.Address of the uplink for the new area. Same format as above.
:pt.:hp2.pchAreaTag:ehp2.
:pd.Area tag of the new area.
:pt.:hp2.ulFlags:ehp2.
:pd.Additional flags. Currently, this parameter is unused and always 0.
:eparml.
:p.
:hp2.Return values:ehp2.
:p.
:sl compact.
:li.ECHOMAN_OK
:li.ECHOMAN_PARAMSIZE
:li.ECHOMAN_FORMAT
:li.ECHOMAN_ERROR
:li.ECHOMAN_CFGNOTFOUND
:li.ECHOMAN_CFGREAD
:li.ECHOMAN_CFGWRITE
:li.ECHOMAN_CFGFORMAT
:li.ECHOMAN_NOTLINKED
:esl.

:h2.Return values
:p.
The following error values may be returned by some of the DLL functions. They are defined in ECHOMAN.H.
:parml break=none tsize=30.
:pt.:hp2.ECHOMAN_OK:ehp2.
:pd.No error occured.
:pt.:hp2.ECHOMAN_PARAMSIZE:ehp2.
:pd.The size of the parameter block passed to the DLL does not match the
required parameter block.
:pt.:hp2.ECHOMAN_FORMAT:ehp2.
:pd.The signature found in the parameter block does not match the
DLL's own signature.
:pt.:hp2.ECHOMAN_CANCEL:ehp2.
:pd.The user canceled the setup dialog. The DLL must leave the data in the
parameter block unmodified.
:pt.:hp2.ECHOMAN_CFGNOTFOUND:ehp2.
:pd.The tosser configuration file was not found or could not be opened.
:pt.:hp2.ECHOMAN_CFGREAD:ehp2.
:pd.An error occured while reading the configuration file.
:pt.:hp2.ECHOMAN_CFGWRITE:ehp2.
:pd.An error occured while writing the configuration file.
:pt.:hp2.ECHOMAN_CFGFORMAT:ehp2.
:pd.The format of the configuration file can not be handled by the DLL.
:pt.:hp2.ECHOMAN_ALREADYLINKED:ehp2.
:pd.While trying to add an echo area to the configuration file, the area
was found to be already present.
:pt.:hp2.ECHOMAN_NOTLINKED:ehp2.
:pd.While trying to remove an echo area from the configuration file, the
area was not found.
:pt.:hp2.ECHOMAN_ERROR:ehp2.
:pd.Other unspecific error.
:eparml.

:h2.Files
:p.
The following files come with this Development Kit&colon.
:parml break=none tsize=20.
:pt.:hp2.ECHOMAN.H:ehp2.
:pd.Definitions and prototypes for writing Echo Manager extension DLLs. The file
can be used directly with IBM CSet++. You may need to modify it for other
compilers.
:pt.:hp2.EXAMPLE.C:ehp2.
:pd.Example Echo Manager extension DLL. This files demonstrates how to use the
DLL interface. No "real" work is done by the DLL. However, it may be used as
a skeleton program for your own DLL.
:eparml.


:fn id=syscall.
:p.This may actually be called differently with your compiler. It's the same
calling convention used to call OS/2 API functions. Configure your compiler
appropriately.
:efn.

:euserdoc.
