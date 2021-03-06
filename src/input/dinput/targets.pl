my @defines;

## compiler flags ##
@defines = qw( AMPLUS USE_DDRAW USE_DINPUT );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}
if (defined($no_asm) && $no_asm) {
  push (@defines, "NO_ASM");
}
## end compiler flags ##

## include paths ##
my @includes = qw( ../../../include );

if (!$disable_wine && defined($wine_prefix)) {
  push (@includes, "$wine_prefix/include/wine/windows",
                   "$wine_prefix/include/wine/msvcrt");
}

if (defined($dxsdk_path)) {
  push (@includes, "$dxsdk_path/include");
}
## end include paths ##

my @targets = qw(
OMOUSE.cpp
);

build_targets(\@targets, \@includes, \@defines);
